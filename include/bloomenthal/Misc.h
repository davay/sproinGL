// Misc.h

#ifndef MISC_HDR
#define MISC_HDR

#include "VecMat.h"

// Misc
std::string GetDirectory();

// Sphere
int LineSphere(vec3 ln1, vec3 ln2, vec3 center, float radius, vec3 &p1, vec3 &p2);
    // set points intersected by line with sphere, return # intersection
    // line defined by ln1, ln2
    // sphere defined by center and radius
    // p1 and p2 set according to # hits; return # hits

float RaySphere(vec3 base, vec3 v, vec3 center, float radius);
    // return least pos alpha of ray and sphere (or -1 if none)
    // v presumed unit length

// Image file
unsigned char *ReadTarga(const char *filename, int &width, int &height);
    // allocate width*height pixels, set them from file, return pointer
    // this memory should be freed by the caller
    // expects 24 bpp
    // *** pixel data is BGR format ***

bool WriteTarga(const char *filename, unsigned char *pixels, int width, int height);
    // save raster to named Targa file

bool WriteTarga(char *filename);
    // as above but with entire application raster

// Texture

// textureUnit is assigned by the programmer: it is the OpenGL texture resource used
//      it specifies the uniform sampler2D and the active texture
// textureName is assigned by OpenGL: it is needed by glBindTexture to bind
//      to an active texture and by glDeleteBuffers

GLuint LoadTexture(const char *targaFilename, GLuint textureUnit, bool mipmap = true);
    // load .tga file as given texture unit; return texture name (ID)

GLuint LoadTexture(unsigned char *pixels, int width, int height, GLuint textureUnit, bool bgr = false, bool mipmap = true);
    // load pixels as given texture unit; return texture name (ID)

void LoadTexture(unsigned char *pixels, int width, int height, GLuint textureUnit, GLuint textureName, bool bgr, bool mipmap);

// Bump map
unsigned char *GetNormals(unsigned char *depthPixels, int &width, int &height, float pixelScale = 25);
    // return normal pixels that correspond with depth pixels
    // this memory should be freed by the caller

#endif
