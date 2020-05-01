// Misc.cpp

#include <glad.h>
#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include "Draw.h"
#include "Misc.h"
#include <unistd.h>

// Misc
std::string GetDirectory() {
    char buf[256];
    getcwd(buf, 256);
    return std::string(buf)+std::string("\\");
}

// Sphere

int LineSphere(vec3 ln1, vec3 ln2, vec3 center, float radius, vec3 &p1, vec3 &p2) {
    // set points intersected by line ln1,ln2 with sphere, return # intersection
    vec3 base = ln1, v = normalize(ln2-ln1);
    vec3 q = base-center;
    float vDot = dot(v, q);
    float sq = vDot*vDot-dot(q, q)+radius*radius;
    if (sq < 0)
        return 0;
    float root = sqrt(sq), a1 = -vDot-root, a2 = -vDot+root;
    p1 = base+a1*v;
    p2 = base+a2*v;
    return root > -FLT_EPSILON && root < FLT_EPSILON? 1 : 2;
}

float RaySphere(vec3 base, vec3 v, vec3 center, float radius) {
    // return least pos alpha of ray and sphere (or -1 if none)
    vec3 q = base-center;
    float vDot = dot(v, q);
    float sq = vDot*vDot-dot(q, q)+radius*radius;
    if (sq < 0)
        return -1;
    float root = sqrt(sq), a = -vDot-root;
    return a > 0? a : -vDot+root;
}

// Image File

unsigned char *ReadTarga(const char *filename, int &width, int &height) {
    // open targa file, read header, return pointer to pixels
    FILE *in = fopen(filename, "rb");
    if (in) {
        short tgaHeader[9];
        fread(tgaHeader, sizeof(tgaHeader), 1, in);
        // allocate, read pixels
        int w = width = tgaHeader[6], h = height = tgaHeader[7];
        int bitsPerPixel = tgaHeader[8], bytesPerPixel = bitsPerPixel/8, bytesPerImage = w*h*bytesPerPixel;
        if (bytesPerPixel != 3) {
            printf("bytes per pixel not 3!\n");
            return NULL;
        }
        unsigned char *pixels = new unsigned char[bytesPerImage];
        fread(pixels, bytesPerImage, 1, in);
        fclose(in);
        return pixels;
    }
    printf("can't open %s\n", filename);
    return NULL;
}

bool WriteTarga(const char *filename, unsigned char *pixels, int width, int height) {
    FILE *out = fopen(filename, "wb");
    if (!out) {
        printf("can't save %s\n", filename);
        return false;
    }
    short tgaHeader[9];
    tgaHeader[0] = 0;
    tgaHeader[1] = 2;
    tgaHeader[2] = 0;
    tgaHeader[3] = 0;
    tgaHeader[4] = 0;
    tgaHeader[5] = 0;
    tgaHeader[6] = width;
    tgaHeader[7] = height;
    tgaHeader[8] = 24; // *** assumed bits per pixel
    fwrite(tgaHeader, sizeof(tgaHeader), 1, out);
    fwrite(pixels, 3*width*height, 1, out);
    fclose(out);
    return true;
}

bool WriteTarga(char *filename) {
    int width, height;
    GetViewportSize(width, height);
    int npixels = width*height;
    unsigned char *cPixels = new unsigned char[3*npixels], *c = cPixels;
    float *fPixels = new float[3*npixels*sizeof(float)], *f = fPixels;
    glReadPixels(0, 0, width, height, GL_BGR, GL_FLOAT, fPixels);   // Targa is BGR ordered
    for (int i = 0; i < npixels; i++) {
        *c++ = (unsigned char) (255.f*(*f++));
        *c++ = (unsigned char) (255.f*(*f++));
        *c++ = (unsigned char) (255.f*(*f++));
    }
    bool ok = WriteTarga(filename, (unsigned char *) cPixels, width, height);
    delete [] fPixels;
    delete [] cPixels;
    return ok;
}

// Texture

void LoadTexture(unsigned char *pixels, int width, int height, GLuint textureUnit, GLuint textureName, bool bgr, bool mipmap) {
    glActiveTexture(GL_TEXTURE0+textureUnit);       // active texture corresponds with textureUnit
    glBindTexture(GL_TEXTURE_2D, textureName);      // bind active texture to textureName
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);          // accommodate width not multiple of 4
    // specify target, format, dimension, transfer data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, bgr? GL_BGR : GL_RGB, GL_UNSIGNED_BYTE, pixels);
    if (mipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            // average bounding pixels of bounding mipmaps - should be default but sometimes needed (else alias)
    }
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

GLuint LoadTexture(unsigned char *pixels, int width, int height, GLuint textureUnit, bool bgr, bool mipmap) {
    GLuint textureName = 0;
    // allocate GPU texture buffer; copy, free pixels
    glGenTextures(1, &textureName);
    LoadTexture(pixels, width, height, textureUnit, textureName, bgr, mipmap);
    return textureName;
}

GLuint LoadTexture(const char *targaFilename, GLuint textureUnit, bool mipmap) {
    int width, height;
    unsigned char *pixels = ReadTarga(targaFilename, width, height);
    GLuint textureName = LoadTexture(pixels, width, height, textureUnit, true, mipmap); // Targa is BGR
    delete [] pixels;
    return textureName;
}

// Bump map

unsigned char *GetNormals(unsigned char *depthPixels, int &width, int &height, float pixelScale) {
    class Helper { public:
        unsigned char *depthPixels, *bumpPixels;
        int width, height;
        float pixelScale;
        float GetDepth(int i, int j) {
            unsigned char *v = depthPixels+3*(j*width+i); // assume r == g == b
            return ((float) *v)/255.f;
        }
        float Dz(int i1, int j1, int i2, int j2) {
            return GetDepth(i2, j2)-GetDepth(i1, j1);
        }
        vec3 Normal(int i, int j) {
            int i1 = i > 0? i-1 : i, i2 = i < width-1? i+1 : i;
            int j1 = j > 0? j-1 : j, j2 = j < height-1? j+1 : j;
            vec3 vx((float)(i2-i1)/pixelScale, 0, Dz(i1, j, i2, j));
            vec3 vy(0, (float)(j2-j1)/pixelScale, Dz(i, j1, i, j2));
            vec3 v = cross(vx, vy);
            v = normalize(v);
            return v.z < 0? -v : v;
        }
        Helper(unsigned char *depthPixels, int width, int height, float pixelScale) :
            depthPixels(depthPixels), width(width), height(height), pixelScale(pixelScale) {
                int bytesPerPixel = 3, bytesPerImage = width*height*bytesPerPixel;
                unsigned char *n = bumpPixels = new unsigned char[bytesPerImage];
                for (int j = 0; j < height; j++)                  // row
                    for (int i = 0; i < width; i++) {             // column
                        vec3 v = Normal(i, j);
                        *n++ = (unsigned char) (127.5f*(v[0]+1)); // red in [-1,1]
                        *n++ = (unsigned char) (127.5f*(v[1]+1)); // grn in [-1,1]
                        *n++ = (unsigned char) (255.f*v[2]);      // blu in [0,1]
                    }
        }
    } h(depthPixels, width, height, pixelScale);
    return h.bumpPixels;
}
