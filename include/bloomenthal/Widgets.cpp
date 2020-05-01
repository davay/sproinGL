// Widgets.cpp

#include <glad.h>
#include <GLFW/glfw3.h>
#include <glu.h>
#include "Draw.h"
#include "GLXtras.h"
#include "Misc.h"
#include "Widgets.h"
#include <float.h>
#include <stdio.h>
#include <string.h>

#define USE_TEXT

#ifdef USE_TEXT
    #include "Text.h"
#endif

// Support

bool MouseOver(double x, double y, vec2 &p, int proximity, int xCursorOffset, int yCursorOffset) {
    return length(vec2((float)(x+xCursorOffset), (float)(y+yCursorOffset))-p) < proximity;
}

bool MouseOver(int x, int y, vec2& p, int proximity, int xCursorOffset, int yCursorOffset) {
    float xo = static_cast<float>(x+xCursorOffset), yo = static_cast<float>(y+yCursorOffset);
    return length(vec2(xo, yo)-p) < proximity;
}

bool MouseOver(double x, double y, vec3 &p, mat4 &view, int proximity, int xCursorOffset, int yCursorOffset) {
    return ScreenDistSq(x+xCursorOffset, y+yCursorOffset, p, view) < proximity*proximity;
}

bool Nil(float f) { return f > -FLT_EPSILON && f < FLT_EPSILON; }

float DotProduct(float a[], float b[]) { return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]; }

double ScreenZ(vec3 p, mat4 m) { return (m*vec4(p, 1)).z; }

bool FrontFacing(vec3 base, vec3 vec, mat4 view) { return ScreenZ(base, view) >= ScreenZ(base+vec, view); }

vec3 ProjectToLine(vec3 &p, vec3 &p1, vec3 &p2) {
    // project p to line p1p2
    vec3 delta(p2-p1);
    if (Nil(delta.x) && Nil(delta.y) && Nil(delta.z))
        return p1;
    vec3 dif(p-p1);
    float alpha = dot(delta, dif)/dot(delta, delta);
    return p1+alpha*delta;
}

vec3 FrameBase(mat4 &f) { return vec3(f[0][3], f[1][3], f[2][3]); }

float FrameScale(mat4 &f) { return length(vec3(f[0][0], f[1][0], f[2][0])); }

void SetBase(mat4 &f, vec3 &base) {
    for (int i = 0; i < 3; i++)
        f[i][3] = base[i];
}

void Scale3x3(mat4 &f, float scale) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            f[i][j] *= scale;
}

// Framer

Framer::Framer() { Set(NULL, 0, mat4(1)); }

Framer::Framer(mat4 *m, float radius, mat4 fullview) { Set(m, radius, fullview); }

void Framer::Set(mat4 *m, float radius, mat4 fullview) {
    vec2 s(0, 0);
    if (m) {
        base = FrameBase(*m);
        s = ScreenPoint(base, fullview);
    }
    arcball.Set(m, s, radius);
    moverPicked = false;
}

bool Framer::Hit(int x, int y) { return arcball.Hit(x, y); }

void Framer::Down(int x, int y, mat4 modelview, mat4 persp) {
    moverPicked = MouseOver(x, y, arcball.center);
    if (moverPicked)
        mover.Down(&base, x, y, modelview, persp);
    else
        arcball.Down(x, y);
}

void Framer::Drag(int x, int y, mat4 modelview, mat4 persp) {
    if (moverPicked) {
        mover.Drag(x, y, modelview, persp);
        SetBase(*arcball.m, base);
        arcball.SetCenter(ScreenPoint(base, persp*modelview));
    }
    else
        arcball.Drag(x, y);
}

void Framer::Up() { arcball.Up(); }

void Framer::Wheel(bool forward, bool shift) { arcball.Wheel(forward, shift); }

void Framer::Draw(mat4 fullview) {
    if (arcball.m) {
        UseDrawShader(ScreenMode());
        arcball.Draw();
        // draw center
        UseDrawShader(fullview);
        Disk(base, 10, vec3(1,0,0));
    }
}

mat4 *Framer::GetMatrix() { return arcball.m; }

// Arcball

Arcball::Arcball() { Set(NULL, vec2(), 0); }

Arcball::Arcball(mat4 *m, vec2 center, float radius) { Set(m, center, radius); }

void Arcball::Set(mat4 *mat, vec2 c, float r) {
    m = mat;
    center = c;
    radius = r;
    scale = 1;
    if (m)
        scale = FrameScale(*m);
}

void Arcball::SetCenter(vec2 c) { center = c; }
void Arcball::SetCenter(vec2 c, float r) { center = c; radius = r; }

bool Arcball::Hit(int x, int y) {
    vec2 dif(x-center.x, y-center.y);
    return dot(dif, dif) < radius*radius;
}

vec3 Arcball::BallV(vec2 mouse) {
    // return point on sphere centered on origin with radius in pixels
    // constrain to circumference if mouse exceeds radius
    vec2 dif(mouse-center);
    float difLen = length(dif);
    if (difLen > .97f*radius)
        dif *= (.97f*radius/difLen);
    float sq = radius*radius-dot(dif, dif);
    vec3 v(dif.x, dif.y, sqrt(sq)); // in pixels
    return normalize(v);
}

void Arcball::Down(int x, int y) {
    mouseDown = mouseMove = vec2((float) x, (float) y);
    if (m)
        qstart = Quaternion(*m);
}

void Arcball::Up() { mouseDown = mouseMove = vec2(0, 0); }

void Arcball::Drag(int x, int y) {
    mouseMove = vec2((float) x, (float) y);
    vec3 v1 = BallV(mouseDown), v2 = BallV(mouseMove);
    vec3 axis = cross(v2, v1);
    if (dot(axis, axis) > .000001f) {
        Quaternion qrot(axis, (float) acos((double) dot(v1, v2)));
        Quaternion q = qq = qstart*qrot;
        mat3 m3 = q.Get3x3();
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                (*m)[i][j] = scale*m3[i][j];
    }
}

void Arcball::Wheel(float direction, bool shift) {
    // **** this doesn't work well with CameraArcball
    scale *= (direction < 0? 1.01f : .99f);
    Scale3x3(*m, scale/FrameScale(*m));
}

mat4 *Arcball::GetMatrix() { return m; }

Quaternion Arcball::GetQ() { return qq; }

void Arcball::Draw() {
    if (m) {
        UseDrawShader(ScreenMode());
        vec3 v1 = BallV(mouseDown), v2 = BallV(mouseMove), s1;
        // draw arc
        if (length(mouseDown-mouseMove) > 2) {
            for (int i = 0; i < 24; i++) {
                vec3 m = v1+((float)i/23)*(v2-v1);
                vec3 v = radius*normalize(m);
                vec3 s2(center.x+v.x, center.y+v.y, 0);
                if (i > 0)
                    Line(s1, s2, 2, vec3(1, .2f, .8f)); // 1,0,0));
                s1 = s2;
            }
        }
        // draw outer circle
        vec3 p1(center.x+radius, center.y, 0), p2;
        for (int i = 1; i < 36; i++) {
            float ang = 2*3.141592f*((float)i/35);
            p2 = vec3(center.x+radius*cos(ang), center.y+radius*sin(ang), 0);
            Line(p1, p2, 2, vec3(1, .2f, .8f)); // 1,0,0));
            p1 = p2;
        }
//      UseDrawShader(fullview); // ?? ***
    }
}

// Mover

void SetPlane(vec3 p, int x, int y, mat4 modelview, mat4 persp, float *plane) {
    for (int i = 0; i < 3; i++)
        plane[i] = modelview[2][i];  // zrow (which is product of modelview matrix with untransformed z-axis)
    plane[3] = -plane[0]*p.x-plane[1]*p.y-plane[2]*p.z; // pass plane through point
}

void Mover::Down(vec3 *p, int x, int y, mat4 modelview, mat4 persp) {
    vec2 s = ScreenPoint(*p, persp*modelview);
    mouseOffset = vec2(s.x-x, s.y-y);
    point = p;
    SetPlane(*p, x, y, modelview, persp, plane);
}

void Mover::Drag(int xMouse, int yMouse, mat4 modelview, mat4 persp) {
    if (!point)
        return;
    vec3 p1, p2, axis;
    float x = xMouse+mouseOffset.x, y = yMouse+mouseOffset.y;
    ScreenLine((float) x, (float) y, modelview, persp, p1, p2);
    // get two points that transform to pixel x,y
    axis = p2-p1;
    // direction of line through p1
    float pdDot = dot(axis, plane);
    // project onto plane normal
    float a = (-plane[3]-dot(p1, plane))/pdDot;
    // intersection of line with plane
    *point = p1+a*axis;
}
/*
void Mover::Drag(int xMouse, int yMouse, mat4 modelview, mat4 persp) {
    if (!point)
        return;
    float p1[3], p2[3], axis[3];
    float x = xMouse+mouseOffset.x, y = yMouse+mouseOffset.y;
    ScreenLine((float) x, (float) y, modelview, persp, p1, p2);
    // get two points that transform to pixel x,y
    for (int i = 0; i < 3; i++)
        axis[i] = p2[i]-p1[i];
    // direction of line through p1
    float pdDot = DotProduct(axis, plane);
    // project onto plane normal
    float a = (-plane[3]-DotProduct(p1, plane))/pdDot;
    // intersection of line with plane
    for (int j = 0; j < 3; j++)
        (*point)[j] = p1[j]+a*axis[j];
} */

bool Mover::Hit(int x, int y, mat4 &view, int proximity) {
    return MouseOver((float) x, (float) y, *point, view, proximity);
}

bool Mover::IsSet(void *p) { return p? point == p : point != NULL; }

void Mover::Unset() { point = NULL; }

Mover::Mover() { }

// Joystick

Joystick::Joystick(vec3 *b, vec3 *v, float arrowScale, vec3 color) :
    base(b), vec(v), arrowScale(arrowScale), color(color) { }

bool Joystick::Hit(int x, int y, mat4 fullview) {
    return ScreenDistSq(x, y, *base, fullview) < 100 ||
           ScreenDistSq(x, y, *base+*vec, fullview) < 100;
}

void Joystick::Down(int x, int y, vec3 *b, vec3 *v, mat4 modelview, mat4 persp) {
    mat4 fullview = persp*modelview;
    base = b;
    vec = v;
    fwdFace = FrontFacing(*base, *vec, fullview);
    mode = ScreenDistSq(x, y, *base, fullview) < 100? JoyType::A_Base :
           ScreenDistSq(x, y, *base+*vec, fullview) < 100? JoyType::A_Tip : JoyType::A_None;
    if (mode == JoyType::A_Base)
        SetPlane(*base, x, y, modelview, persp, plane);
}

void Joystick::Drag(int x, int y, mat4 modelview, mat4 persp) {
    vec3 p1, p2;                                        // p1p2 is world-space line that xforms to line perp to screen at (x, y)
    ScreenLine((float) x, (float) y, modelview, persp, p1, p2);
    if (mode == JoyType::A_Base) {
        vec3 axis(p2-p1);                               // direction of line through p1
        vec3 normal(plane[0], plane[1], plane[2]);
        float pdDot = dot(axis, normal);
        float a = (-plane[3]-dot(p1, normal))/pdDot;    // project onto plane normal
        *base = p1+a*axis;                              // intersection of line with plane
    }
    if (mode == JoyType::A_Tip) {
        float len = length(*vec);
        vec3 hit1, hit2;
        int nhits = LineSphere(p1, p2, *base, len, hit1, hit2);
        if (nhits == 0) {
            vec3 n = ProjectToLine(*base, p1, p2);
            vec3 v = normalize(n-*base);
            *vec = len*v;
        }
        else {
            *vec = hit2-*base;
            if (FrontFacing(*base, *vec, modelview) != fwdFace)
                *vec = hit1-*base;
        }
    }
}

void Joystick::Draw(vec3 color, mat4 modelview, mat4 persp) {
    bool frontFacing = FrontFacing(*base, *vec, modelview);
    UseDrawShader(persp*modelview);
#ifdef GL_LINE_STIPPLE
    if (!frontFacing) {
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(6, 0xAAAA);
    }
#endif
//  ArrowV(base, len*v, modelview, color, NULL, 10);
    Line(*base, *base+*vec, 5, color);
#ifdef GL_LINE_STIPPLE
    glDisable(GL_LINE_STIPPLE);
#endif
    Disk(*base, 12, vec3(1,0,1));
    Disk(*base+*vec, 12, vec3(1,0,1));
}

void Joystick::SetVector(vec3 v) { *vec = v; }

void Joystick::SetBase(vec3 b) { *base = b; }

// Toggler

Toggler::Toggler(bool *on, const char *name, int x, int y, float dia, vec3 onCol, vec3 offCol, vec3 ringCol)
    : on(on), name(name), x(x), y(y), dia(dia), onCol(onCol), offCol(offCol), ringCol(ringCol) { };

void Toggler::Draw() {
    // assume ScreenMode and no depth-test
    vec3 p((float)x, (float)y, 0);
    Disk(p, dia, ringCol);
    Disk(p, dia-6, *on? onCol : offCol);
#ifdef USE_TEXT
    Text(x+10, y-6, vec3(0,0,0), 10, name.c_str());
#endif
}

bool Toggler::Hit(int xMouse, int yMouse, int proximity) {
    vec2 p((float)x, (float)y);
    return MouseOver(xMouse, yMouse, p, proximity);
}

bool Toggler::UpHit(int xMouse, int yMouse, int state, int proximity) {
    bool hit = Hit(xMouse, yMouse, proximity);
    if (state == GLFW_RELEASE && hit)
        *on = *on? false : true;
    return hit;
}

bool Toggler::On() { return *on; }

const char *Toggler::Name() { return name.c_str(); }

// Magnifier

Magnifier::Magnifier(int2 srcLoc, int2 dspSize, int blockSize) : srcLoc(srcLoc), blockSize(blockSize) {
    int nxBlocks = dspSize[0]/blockSize, nyBlocks = dspSize[1]/blockSize;
    displaySize = int2(nxBlocks*blockSize, nyBlocks*blockSize);
}

void Magnifier::Down(int x, int y) {
    srcLocSave = srcLoc;
    mouseDown = int2(x, y);
}

void Magnifier::Drag(int x, int y) {
    srcLoc[0] = srcLocSave[0]+x-mouseDown[0];
    srcLoc[1] = srcLocSave[1]+y-mouseDown[1];
}

bool Magnifier::Hit(int x, int y) {
    int nxBlocks = displaySize[0]/blockSize, nyBlocks = displaySize[1]/blockSize;
    return x >= srcLoc[0] && y >= srcLoc[1] && x <= srcLoc[0]+nxBlocks-1 && y <= srcLoc[1]+nyBlocks-1;
}

void Magnifier::Display(int2 displayLoc, bool showSrcWindow) {
    class Helper { public:
        void Rect(int xi, int yi, int wi, int hi, bool solid, vec3 col) {
            float x = (float) xi, y = (float) yi, w = (float) wi, h = (float) hi;
            vec3 p0(x, y, 0), p1(x+w, y, 0), p2(x+w, y+h, 0), p3(x, y+h, 0);
            Quad(p0, p1, p2, p3, solid, col, 1, 2);
        }
    } h;
    int nxBlocks = displaySize[0]/blockSize, nyBlocks = displaySize[1]/blockSize;
    int dy = displaySize[1]-nyBlocks*blockSize;
    float *pixels = new float[nxBlocks*nyBlocks*3];
    glReadPixels(srcLoc[0], srcLoc[1], nxBlocks, nyBlocks, GL_RGB, GL_FLOAT, pixels);
    for (int i = 0; i < nxBlocks; i++)
        for (int j = 0; j < nyBlocks; j++) {
            float *pixel = pixels+3*(i*nyBlocks+j);
            vec3 col(pixel[0], pixel[1], pixel[2]);
            h.Rect(displayLoc[0]+blockSize*i, displayLoc[1]+blockSize*j+dy, blockSize, blockSize, true, col);
        }
    glDisable(GL_BLEND);
    if (showSrcWindow)
        h.Rect(srcLoc[0], srcLoc[1], nxBlocks-1, nyBlocks-1, false, vec3(0, 1, 1));
    h.Rect(displayLoc[0], displayLoc[1]+dy, nxBlocks*blockSize, nyBlocks*blockSize, false, vec3(0, 1, 1));
    delete [] pixels;
}
