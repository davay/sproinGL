// Numbers.cpp - display non-negative integers as grayscale text

#include <glad.h>    // gl
#include "Draw.h"    // ScreenPoint
#include <cstring>
#include "GLXtras.h" // LinkProgramViaCode, VertexAttribPointer, SetUniform
#include "Misc.h"    // LoadTexture
#include "Numbers.h" // Number
#include <stdio.h>   // printf

namespace {

// next 10 lines each with 82 grayscale values, each value represented as two hexadecimal characters
const char *image = "\
FFEC7A110034C3FFFFDD98340047FFFFFFFFA723002389FFFFFF000000117AFFFFFFFFFFC3000089FFFF890000000000FFFFFFFFA734000089FF89000000000000C3FFDD57000023A7FFFFEC69110034B5FF\
FF470000000011DDFF5700000047FFFFFF980000000000B5FFFF000000000098FFFFFFFF23000089FFFF890000000000FFFFFF690000000089FF89000000000000C3FF340000000000D0FF470000000000DD\
D00011DDFF690069FF987AB50047FFFFFFEC47DDFF89007AFFFFFFFFFF980047FFFFFF89007A0089FFFF8900C3FFFFFFFFFFC30023B5FFFFFFFFFFFFFFFFEC1123FFFF0034ECFFA700C3DD0023DDFF890089\
890098FFC3340034FFFFFFC30047FFFFFFFFFFFFFFB50089FFFFFFFFFF890089FFFFDD0057C30089FFFF8900C3FFFFFFFFFF7A00B5FFFFFFFFFFFFFFFFFF890089FFFF2311B5DD3411ECC30023ECFFA70047\
8900A74700000000FFFFFFC30047FFFFFFFFFFFFFF5700C3FFFFFF00000057ECFFFF4711DDC30089FFFF8900000034B5FFFF47007A230034D0FFFFFFFFEC1111ECFFFFD023000000D0FFFF11000000000047\
8900000023B50000FFFFFFC30047FFFFFFFFFFFF980069FFFFFFFF0000000069FFB50098FFC30089FFFF890000000000C3FF47000000000011FFFFFFFF890089FFFFFFB51123230034ECFFD03400237A0047\
89001189FFDD0047FFFFFFC30047FFFFFFFFFF980069FFFFFFFFFFFFFFC31100DD4700000000000000FFFFFFFFEC7A0089FF470069ECEC3400C3FFFFEC1111ECFFFFEC1123ECEC570098FFFFFFFFFFB5007A\
B50034DDEC57007AFFFFFFC30047FFFFFFFF690089FFFFFFFFFFFFFFFFDD2300DD4700000000000000FFFFFFFFEC690089FF8900A7FFDD2300D0FFFF890089FFFFFFC30034FFFFA70089FFFFFFFFB52300C3\
FF340000000011DDFF47000000000047FF98000000000000FFC3000000000047FFFFFFFFFFC30089FFFF470000000011DDFFDD000000000047FFFFEC1111ECFFFFFFEC110000000000C3FF470000000069FF\
FFEC69000057D0FFFF47000000000047FF89000000000000FFC30000003489ECFFFFFFFFFFC30089FFFF4700001169DDFFFFFFB534001169ECFFFF890089FFFFFFFFFFC334000034B5FFFF4700003498FFFF";

// transform 2D vertex by view, separate uv from vec4
const char *vertexShader = R"(
    #version 130
    in vec4 point;
    out vec2 vUv;
    uniform mat4 view;
    void main() {
        gl_Position = view*vec4(point.xy, 0, 1);
        vUv = point.zw;
    }
)";

// opacity depends on darkness of texture map
const char *pixelShader = R"(
    #version 130
    in vec2 vUv;
    out vec4 pColor;
    uniform sampler2D textureImage;
    uniform vec3 color;
    void main() {
        float a = texture(textureImage, vUv).r;
        pColor = vec4(color, 1-a);
    }
)";

GLuint shaderProgram = 0, vertexBuffer = 0, textureName = 0;
int textureUnit = 0;

} // end namespace

void Number(int x, int y, unsigned int n, vec3 color, float ptSize) {
    if (!textureName) {
        // create and load texture raster
        int nchars = strlen(image), npixels = nchars/2, height = 10, width = npixels/height;
        unsigned char *pixels = new unsigned char[3*npixels], *p = pixels, *n = (unsigned char *) image;
        for (int i = 0; i < npixels; i++) {
            char c1 = *n++, c2 = *n++;
            int k1 = c1 < 58? c1-'0' : 10+c1-'A', k2 = c2 < 58? c2-'0' : 10+c2-'A', b = 16*k1+k2;
            for (int k = 0; k < 3; k++)
                *p++ = (unsigned char) b;
        }
        textureName = LoadTexture(pixels, width, height, textureUnit);
        delete [] pixels;
    }
    if (!textureName)
        printf("can't make numbers texture map\n");
    if (!shaderProgram)
        shaderProgram = LinkProgramViaCode(&vertexShader, &pixelShader);
    glUseProgram(shaderProgram);
    if (!vertexBuffer) {
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*6, NULL, GL_DYNAMIC_DRAW);
			// need 4 vertices for quad, but 6 if triangles
    }
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    VertexAttribPointer(shaderProgram, "point", 4, 4*sizeof(float), 0);
		// each vertex is 4 floats, stride is 4 floats
    glBindTexture(GL_TEXTURE_2D, textureName);
    // set screen-mode
    SetUniform(shaderProgram, "view", ScreenMode());
    // set text color and texture map, activate texture
    SetUniform(shaderProgram, "color", color);
//  GLint tid = glGetUniformLocation(shaderProgram, "textureImage");
//  if (tid >= 0)
//      glUniform1ui(tid, textureName);
    SetUniform(shaderProgram, "textureImage", textureUnit);
    glActiveTexture(GL_TEXTURE0+textureUnit);
    // enable blended overwrite of color buffer
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // convert number to string, display each digit
    int ndigits = n? 1+(int)log10(n) : 1;
    for (int k = 0; k < ndigits; k++) {
        // value of digit determines horizontal position along texture
        int v = (n / (int) (pow(10, (ndigits-k-1)))) % 10;
        float xx = x+.8f*ptSize*k, yy = (float) y, w = .8f*ptSize, h = ptSize, t = (float)v/10;
        // display each digit as one quad or two triangles, mapped to 1/10 width of texture map
#ifdef GL_QUADS
        float vertices[][4] = {{xx, yy, t, 1}, {xx+w, yy, t+.1f, 1}, {xx+w, yy+h, t+.1f, 0}, {xx, yy+h, t, 0}};
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_QUADS, 0, 4);
#else
        float vertices[][4] = {{xx, yy, t, 1}, {xx+w, yy, t+.1f, 1},   {xx+w, yy+h, t+.1f, 0},
							   {xx, yy, t, 1}, {xx+w, yy+h, t+.1f, 0}, {xx, yy+h, t, 0}};
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
#endif
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Number(vec3 p, mat4 m, unsigned int n, vec3 color, float ptSize) {
    vec2 pp = ScreenPoint(p, m);
    Number((int) pp.x, (int) pp.y, n, color, ptSize);
}
