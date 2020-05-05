// Draw.cpp - various draw operations

#include <glad.h>
#include <glu.h>
#include "Draw.h"
#include "GLXtras.h"
#include "Misc.h"
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Screen Mode

void GetViewportSize(int &width, int &height) {
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    width = vp[2];
    height = vp[3];
}

mat4 Viewport() {
    // map +/-1 space to screen space viewport
    float vp[4];
    glGetFloatv(GL_VIEWPORT, vp);
    float x = vp[0], y = vp[1], w = vp[2], h = vp[3];
    return mat4(vec4(w/2,0,0,x+w/2), vec4(0,h/2,0,y+h/2), vec4(0,0,1,0), vec4(0,0,0,1));
}

mat4 ScreenMode() {
    float vp[4];
    glGetFloatv(GL_VIEWPORT, vp);
    mat4 scale = Scale(2.f/(float)vp[2], 2.f/(float)vp[3], 1);
    mat4 tran = Translate(vp[0]-1, vp[1]-1, 0);
    return tran*scale;
}

bool IsVisible(vec3 p, mat4 fullview, vec2 *screenA, int *w, int *h, float fudge) {
    int width, height;
    if (w && h) {
        width = *w;
        height = *h;
    }
    else
        GetViewportSize(width, height);
    vec4 xp = fullview*vec4(p, 1);
    vec2 clip(xp.x/xp.w, xp.y/xp.w);    // clip space, +/-1
    vec2 screen(((float) width/2.f)*(1.f+clip.x), ((float) height/2.f)*(1.f+clip.y));
    if (screenA)
        *screenA = screen;
    float z = xp.z/xp.w, zScreen;
    glReadPixels((int)screen.x, (int)screen.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zScreen);
        // **** this is slow when used during rendering!
    zScreen = 2*zScreen-1; // seems to work (clip range +/-1 but zbuffer range 0-1)
    return z < zScreen+fudge;
}

vec2 ScreenPoint(vec3 p, mat4 m, float *zscreen) {
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    vec4 xp = m*vec4(p, 1);
    if (zscreen)
        *zscreen = xp.z; // /xp.w;
    return vec2(vp[0]+((xp.x/xp.w)+1)*.5f*(float)vp[2], vp[1]+((xp.y/xp.w)+1)*.5f*(float)vp[3]);
}

float ScreenDistSq(int x, int y, vec3 p, mat4 m, float *zscreen) {
    vec2 screen = ScreenPoint(p, m, zscreen);
    float dx = x-screen.x, dy = y-screen.y;
    return dx*dx+dy*dy;
}

float ScreenDistSq(double x, double y, vec3 p, mat4 m, float *zscreen) {
    vec2 screen = ScreenPoint(p, m, zscreen);
    double dx = x-screen.x, dy = y-screen.y;
    return static_cast<float>(dx*dx+dy*dy);
}

void ScreenRay(float xscreen, float yscreen, mat4 modelview, mat4 persp, vec3 &p, vec3 &v) {
    // compute ray from p in direction v; p is transformed eyepoint, xscreen, yscreen determine v
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    // origin of ray is always eye (translated origin)
    p = vec3(modelview[0][3], modelview[1][3], modelview[2][3]);
    // create transposes for gluUnproject
    double tpersp[4][4], tmodelview[4][4], a[3], b[3];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            tmodelview[i][j] = modelview[j][i];
            tpersp[i][j] = persp[j][i];
        }
    // un-project two screen points of differing depth to determine v
    if (gluUnProject(xscreen, yscreen, .25, (const double*) tmodelview, (const double*) tpersp, vp, &a[0], &a[1], &a[2]) == GL_FALSE)
        printf("UnProject false\n");
    if (gluUnProject(xscreen, yscreen, .50, (const double*) tmodelview, (const double*) tpersp, vp, &b[0], &b[1], &b[2]) == GL_FALSE)
        printf("UnProject false\n");
    v = normalize(vec3((float) (b[0]-a[0]), (float) (b[1]-a[1]), (float) (b[2]-a[2])));
}

void ScreenLine(float xscreen, float yscreen, mat4 modelview, mat4 persp, vec3 &p1, vec3 &p2) {
    // compute 3D world space line, given by p1 and p2, that transforms
    // to a line perpendicular to the screen at (xscreen, yscreen)
    double tpersp[4][4], tmodelview[4][4], a[3], b[3];
    // get viewport
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    // create transposes
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            tmodelview[i][j] = modelview[j][i];
            tpersp[i][j] = persp[j][i];
        }
    if (gluUnProject(xscreen, yscreen, .25, (const double*) tmodelview, (const double*) tpersp, vp, &a[0], &a[1], &a[2]) == GL_FALSE)
        printf("UnProject false\n");
    if (gluUnProject(xscreen, yscreen, .50, (const double*) tmodelview, (const double*) tpersp, vp, &b[0], &b[1], &b[2]) == GL_FALSE)
        printf("UnProject false\n");
        // alternatively, a seond point can be determined by transforming the origin by the inverse of modelview
        // this would yield in world space the camera location, through which all view lines pass
    p1 = vec3(static_cast<float>(a[0]), static_cast<float>(a[1]), static_cast<float>(a[2]));
    p2 = vec3(static_cast<float>(b[0]), static_cast<float>(b[1]), static_cast<float>(b[2]));
}

// Draw Shader

int drawShader = 0;

const char *drawVShader = R"(
    #version 410 core
    in vec3 position;
    in vec3 color;
    out vec3 vColor;
    uniform mat4 view;
    void main() {
        gl_Position = view*vec4(position, 1);
        vColor = color;
    }
)";

const char *drawPShader = R"(
    #version 410 core
    in vec3 vColor;
    out vec4 pColor;
    uniform float opacity = 1;
    uniform int fadeToCenter = 0;
    float Fade(float t) {
        if (t < .95) return 1;
        if (t > 1.05) return 0;
        float a = (t-.95)/(1.05-.95);
        return 1-smoothstep(0, 1, a);
            // unsure if smoothstep helps
    }
    float DistanceToCenter() {
        float dx = 1-2*gl_PointCoord.x;
        float dy = 1-2*gl_PointCoord.y;
        return sqrt(dx*dx+dy*dy);
      }
    void main() {
        // GL_POINT_SMOOTH deprecated, so calc here
        // needs GL_POINT_SPRITE enabled
        float o = opacity;
        if (fadeToCenter == 1)
            o *= Fade(DistanceToCenter());
        pColor = vec4(vColor, o);
    }
)";

int UseDrawShader() {
    int was = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &was);
    bool init = !drawShader;
    if (init)
        drawShader = LinkProgramViaCode(&drawVShader, &drawPShader);
    glUseProgram(drawShader);
    if (init)
        SetUniform(drawShader, "view", mat4());
    return was;
}

int UseDrawShader(mat4 viewMatrix) {
    int was = UseDrawShader();
    SetUniform(drawShader, "view", viewMatrix);
    return was;
}

// Disks

GLuint diskBuffer = -1;

void Disk(vec3 p, float diameter, vec3 color, float opacity) {
    // diameter should be >= 0, <= 20
    UseDrawShader();
    // create buffer for single vertex (x,y,z,r,g,b)
    if (diskBuffer < 0) {
        glGenBuffers(1, &diskBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, diskBuffer);
        int bufferSize = 6*sizeof(float);
        glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, diskBuffer); // set active buffer
    // allocate buffer memory and load location and color data
    glBufferData(GL_ARRAY_BUFFER, 6*sizeof(float), NULL, GL_STATIC_DRAW); // *** needed??
    glBufferSubData(GL_ARRAY_BUFFER, 0, 3*sizeof(float), &p.x);
    glBufferSubData(GL_ARRAY_BUFFER, 3*sizeof(float), 3*sizeof(float), &color.x);
    // connect shader inputs
    VertexAttribPointer(drawShader, "position", 3, 0, (void *) 0);
    VertexAttribPointer(drawShader, "color", 3, 0, (void *) sizeof(vec3));
    // draw
    SetUniform(drawShader, "opacity", opacity);
    glPointSize(diameter);
#if defined(GL_POINT_SMOOTH)
    glEnable(GL_POINT_SMOOTH);
#elseif defined(GL_POINT_SPRITE)
    glEnable(GL_POINT_SPRITE);
#else
    glEnable(0x8861); // same as GL_POINT_SMOOTH [this is a 4.5 core bug]
    SetUniform(drawShader, "fadeToCenter", 1); // needed if GL_POINT_SMOOTH and GL_POINT_SPRITE fail
#endif
    glDrawArrays(GL_POINTS, 0, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Lines

GLuint lineBuffer = -1;

void Line(vec3 p1, vec3 p2, float width, vec3 col1, vec3 col2, float opacity) {
    UseDrawShader();
    // create a vertex buffer for the array
    vec3 data[] = {p1, p2, col1, col2};
    if (lineBuffer <= 0)
        glGenBuffers(1, &lineBuffer);
    // set active vertex buffer, load location and color data
    glBindBuffer(GL_ARRAY_BUFFER, lineBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    // connect shader inputs, set uniforms
    VertexAttribPointer(drawShader, "position", 3, 0, (void *) 0);
    VertexAttribPointer(drawShader, "color", 3, 0, (void *) (2*sizeof(vec3))); // sizeof(points));
    SetUniform(drawShader, "fadeToCenter", 0);  // gl_PointCoord fails for lines (instead, use GL_LINE_SMOOTH)
    SetUniform(drawShader, "opacity", opacity);
    // draw
    glLineWidth(width);
    glDrawArrays(GL_LINES, 0, 2);
    // cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Line(vec3 p1, vec3 p2, float width, vec3 col, float opacity) {
    Line(p1, p2, width, col, col, opacity);
}

void Line(vec2 p1, vec2 p2, float width, vec3 col1, vec3 col2, float opacity) {
    vec3 pp1(p1, 0), pp2(p2, 0);
    Line(pp1, pp2, width, col1, col2, opacity);
}

void Line(vec2 p1, vec2 p2, float width, vec3 col, float opacity) {
    Line(p1, p2, width, col, col, opacity);
}

void Line(int x1, int y1, int x2, int y2, float width, vec3 col, float opacity) {
    vec3 p1((float) x1, (float) y1, 0), p2((float) x2, (float) y2, 0);
    Line(p1, p2, width, col, col, opacity);
}

GLuint lineStripBuffer = 0;

void LineStrip(int nPoints, vec3 *points, vec3 &color, float opacity, float width) {
    if (!lineStripBuffer)
        glGenBuffers(1, &lineStripBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, lineStripBuffer);
    int pSize = nPoints*sizeof(vec3);
    std::vector<vec3> colors(nPoints, color);
    glBufferData(GL_ARRAY_BUFFER, 2*pSize, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, pSize, points);
    glBufferSubData(GL_ARRAY_BUFFER, pSize, pSize, &colors[0]);
    VertexAttribPointer(drawShader, "position", 3, 0, (void *) 0);
    VertexAttribPointer(drawShader, "color", 3, 0, (void *) pSize);
    SetUniform(drawShader, "fadeToCenter", 0);
    SetUniform(drawShader, "opacity", opacity);
    glLineWidth(width);
    glDrawArrays(GL_LINE_STRIP, 0, nPoints);
}

// Quads

GLuint quadBuffer = 0;

void Quad(vec3 p1, vec3 p2, vec3 p3, vec3 p4, bool solid, vec3 col, float opacity, float lineWidth) {
#ifndef GL_QUADS
	Triangle(p1, p2, p3, col, col, col, opacity, !solid, col, lineWidth);
	Triangle(p1, p3, p4, col, col, col, opacity, !solid, col, lineWidth);
#else
    vec3 data[] = { p1, p2, p3, p4, col, col, col, col };
    UseDrawShader();
    if (quadBuffer == 0) {
        glGenBuffers(1, &quadBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(data), NULL, GL_STATIC_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    VertexAttribPointer(drawShader, "position", 3, 0, (void *) 0);
    VertexAttribPointer(drawShader, "color", 3, 0, (void *) (4*sizeof(vec3)));
    SetUniform(drawShader, "opacity", opacity);
    SetUniform(drawShader, "fadeToCenter", 0);
    glLineWidth(lineWidth);
    glDrawArrays(solid? GL_QUADS : GL_LINE_LOOP, 0, 4);
#endif
}

// Arrows

void Arrow(vec2 base, vec2 head, vec3 col, float lineWidth, double headSize) {
    Line(base, head, lineWidth, col);
    if (headSize > 0) {
        vec2 v1 = (float)headSize*normalize(head-base), v2(v1.y/2.f, -v1.x/2.f);
        vec2 head1(head-v1+v2), head2(head-v1-v2);
        Line(head, head1, lineWidth, col);
        Line(head, head2, lineWidth, col);
    }
}

vec3 ProjectToLine(vec3 p, vec3 p1, vec3 p2) {
    // project p to line p1p2
    vec3 delta(p2-p1);
    float magSq = dot(delta, delta);
    float alpha = magSq > FLT_EPSILON? dot(delta, p-p1)/magSq : 0;
    return p1+alpha*delta;
}

void PointScreen(vec3 p, vec2 s, mat4 modelview, mat4 persp, float lineWidth, vec3 col) {
    // draw between p and the nearest point on a 3D screen line through s
    vec3 p1, p2;
    ScreenLine(s.x, s.y, modelview, persp, p1, p2);
    vec3 pp = ProjectToLine(p, vec3(p1), vec3(p2));
    Line(p, pp, lineWidth, col);
}

void ArrowV(vec3 base, vec3 v, mat4 modelview, mat4 persp, vec3 col, float lineWidth, double headSize) {
    mat4 m = persp*modelview;
    vec3 head(base+v);
    vec2 base2 = ScreenPoint(base, m), head2 = ScreenPoint(head, m);
    vec2 v1 = (float)headSize*normalize(head2-base2), v2(v1.y/2.f, -v1.x/2.f);
    vec2 h1(head2-v1+v2), h2(head2-v1-v2);
    // could draw in screen mode, using base2, head2, h1, & h2, but prefer draw in 3D (allows for depth test)
    UseDrawShader(m);
    Line(base, head, lineWidth, col);
    PointScreen(head, h1, modelview, persp, lineWidth, col);
    PointScreen(head, h2, modelview, persp, lineWidth, col);
}

// Triangles with optional outline

GLuint triShader = 0, triBuffer = 0;

// vertex shader
const char *triVShaderCode = R"(
    #version 330 core
    in vec3 point;
    in vec3 color;
    out vec3 vColor;
    uniform mat4 view;
    void main() {
        gl_Position = view*vec4(point, 1);
        vColor = color;
    }
)";

// geometry shader with line-drawing
const char *triGShaderCode = R"(
    #version 330 core
    layout (triangles) in;
    layout (triangle_strip, max_vertices = 3) out;
    in vec3 vPoint[];
    in vec3 vColor[];
    out vec3 gColor;
    noperspective out vec3 gEdgeDistance;
    uniform mat4 viewptM;
    vec3 ViewPoint(int i) {
        return vec3(viewptM*(gl_in[i].gl_Position/gl_in[i].gl_Position.w));
    }
    void main() {
        float ha = 0, hb = 0, hc = 0;
        // transform each vertex into viewport space
        vec3 p0 = ViewPoint(0), p1 = ViewPoint(1), p2 = ViewPoint(2);
        // find altitudes ha, hb, hc
        float a = length(p2-p1), b = length(p2-p0), c = length(p1-p0);
        float alpha = acos((b*b+c*c-a*a)/(2.*b*c));
        float beta = acos((a*a+c*c-b*b)/(2.*a*c));
        ha = abs(c*sin(beta));
        hb = abs(c*sin(alpha));
        hc = abs(b*sin(alpha));
        // send triangle vertices and edge distances
        vec3 edgeDists[3] = { vec3(ha, 0, 0), vec3(0, hb, 0), vec3(0, 0, hc) };
        for (int i = 0; i < 3; i++) {
            gEdgeDistance = edgeDists[i];
            gColor = vColor[i];
            gl_Position = gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
)";

// pixel shader
const char *triPShaderCode = R"(
    #version 410 core
    in vec3 gColor;
    noperspective in vec3 gEdgeDistance;
    uniform vec4 outlineColor = vec4(0, 0, 0, 1);
    uniform float opacity = 1;
    uniform float outlineWidth = 1;
    uniform float transition = 1;
    uniform int outlineOn = 1;
    out vec4 pColor;
    void main() {
        pColor = vec4(gColor, opacity);
        if (outlineOn > 0) {
            float minDist = min(gEdgeDistance.x, min(gEdgeDistance.y, gEdgeDistance.z));
            float t = smoothstep(outlineWidth-transition, outlineWidth+transition, minDist);
            if (outlineOn == 2) pColor = vec4(1,1,1,1);
            pColor = mix(outlineColor, pColor, t);
        }
    }
)";

 void UseTriangleShader() {
    bool init = triShader == 0;
    if (init)
        triShader = LinkProgramViaCode(&triVShaderCode, NULL, NULL, &triGShaderCode, &triPShaderCode);
    glUseProgram(triShader);
    if (init)
        SetUniform(triShader, "view", mat4());
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
}

void UseTriangleShader(mat4 view) {
    UseTriangleShader();
    SetUniform(triShader, "view", view);
}

void Triangle(vec3 p1, vec3 p2, vec3 p3, vec3 c1, vec3 c2, vec3 c3,
              float opacity, bool outline, vec4 outlineCol, float outlineWidth, float transition) {
    vec3 data[] = { p1, p2, p3, c1, c2, c3 };
    UseTriangleShader();
    if (triBuffer == 0)
        glGenBuffers(1, &triBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, triBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    VertexAttribPointer(triShader, "point", 3, 0, (void *) 0);
    VertexAttribPointer(triShader, "color", 3, 0, (void *) (3*sizeof(vec3)));
    SetUniform(triShader, "viewptM", Viewport());
    SetUniform(triShader, "opacity", opacity);
    SetUniform(triShader, "outlineOn", outline? 1 : 0);
    SetUniform(triShader, "outlineWidth", outlineWidth);
    SetUniform(triShader, "transition", transition);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
