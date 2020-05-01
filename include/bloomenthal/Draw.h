// Draw.h

#ifndef DRAW_HDR
#define DRAW_HDR

#include "VecMat.h"

// screen operations
void GetViewportSize(int &width, int &height);
mat4 Viewport();
    // create matrix to map NDC space to pixel space, inverse of ScreenMode
mat4 ScreenMode();
    // create matrix to map pixel space, (0,0)-(width,height), to NDC (clip) space, (-1,-1)-(1,1)
bool IsVisible(vec3 p, mat4 fullview, vec2 *screen = NULL, int *w = NULL, int *h = NULL, float fudge = 0);
    // if the depth test is enabled, is point p visible?
    // if non-null, set screen location (in pixels) of transformed p
    // **** this is slow when used during rendering!
vec2 ScreenPoint(vec3 p, mat4 m, float *zscreen = NULL);
    // transform 3D point to location (xscreen, yscreen), in pixels; if non-null, set zscreen
    // uses current GL viewport
void ScreenRay(float xscreen, float yscreen, mat4 modelview, mat4 persp, vec3 &p, vec3 &v);
void ScreenLine(float xscreen, float yscreen, mat4 modelview, mat4 persp, vec3 &p1, vec3 &p2);
    // compute 3D world space line, given by p1 and p2, that transforms
    // to a line perpendicular to the screen at pixel (xscreen, yscreen)
    // uses current viewport
float ScreenDistSq(int x, int y, vec3 p, mat4 m, float *zscreen = NULL);
float ScreenDistSq(double x, double y, vec3 p, mat4 m, float *zscreen = NULL);
    // return distance squared, in pixels, between screen point (x, y) and point p xformed by view matrix

// 2D/3D drawing functions
int UseDrawShader();
    // invoke shader for Disk, Line, Quad, and Arrow, but do not change view transformation
    // return previous shader ID
int UseDrawShader(mat4 viewMatrix);
    // as above, but set view transformation
void Disk(vec3 p, float diameter, vec3 color, float opacity = 1);
void Line(vec3 p1, vec3 p2, float width, vec3 col, float opacity = 1);
void Line(vec3 p1, vec3 p2, float width, vec3 col1, vec3 col2, float opacity = 1);
void Line(vec2 p1, vec2 p2, float width, vec3 col, float opacity = 1);
void Line(vec2 p1, vec2 p2, float width, vec3 col1, vec3 col2, float opacity = 1);
void Line(int x1, int y1, int x2, int y2, float width, vec3 col, float opacity = 1);
void LineStrip(int nPoints, vec3 *points, vec3 &color, float opacity, float width);
void Quad(vec3 pnt1, vec3 pnt2, vec3 pnt3, vec3 pnt4, bool solid, vec3 color, float opacity = 1, float lineWidth = 1);
void Arrow(vec2 base, vec2 head, vec3 color, float lineWidth = 1, double headSize = 4);
    // display an arrow between base and head
void ArrowV(vec3 base, vec3 v, mat4 modelview, mat4 persp, vec3 color, float lineWidth = 1, double headSize = 4);
    // as above but vector and base are 3D, transformed by m

// triangle operations
void UseTriangleShader();
void UseTriangleShader(mat4 viewMatrix);
void Triangle(vec3 p1, vec3 p2, vec3 p3, vec3 c1, vec3 c2, vec3 c3,
              float opacity = 1, bool outline = false,
              vec4 outlineCol = vec3(0,0,0), float outlineWidth = 1, float transition = 1);

#endif
