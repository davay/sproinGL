// Widgets.h: Cursor Support, Mover, Joystick, Arcball, Framer, Toggler, Magnifier

#ifndef WIDGETS_HDR
#define WIDGETS_HDR

#include <cstring>
#include "Quaternion.h"
#include "VecMat.h"

// Cursor Support

bool MouseOver(double x, double y, vec2 &p, int proximity = 12, int xCursorOffset = 0, int yCursorOffset = 0);
    // is mouse(x,y) within proximity pixels of screen point p?
    // (xCursorOffset, yCursorOffset) added to (x, y) to compensate for arrow tip location

bool MouseOver(int x, int y, vec2& p, int proximity = 12, int xCursorOffset = 0, int yCursorOffset = 0);

bool MouseOver(double x, double y, vec3 &p, mat4 &view, int proximity = 12, int xCursorOffset = 0, int yCursorOffset = 0);

// Mover: direct move of selected point, along plane perpendicular to camera

class Mover {
public:
    Mover();
    void Down(vec3 *p, int x, int y, mat4 modelview, mat4 persp);
    void Drag(int x, int y, mat4 modelview, mat4 persp);
    bool Hit(int x, int y, mat4 &view, int proximity = 12);
    bool IsSet(void *p = NULL);
        // return true if Mover::point has been set; if p non-null, return true if point==p
    void Unset();
private:
    vec3 *point = NULL;
    float plane[4] = {0, 0, 0, 0}; // unnormalized
    vec2  mouseOffset;
friend class Framer;
};

// Joystick: adjust selected vector, along sphere centered at vector base

enum class JoyType {A_None, A_Base, A_Tip};

class Joystick {
public:
    Joystick() { };
    Joystick(vec3 *b, vec3 *v, float arrowScale = 1, vec3 color = vec3(0,0,0));
    bool Hit(int x, int y, mat4 fullview);
    void Down(int x, int y, vec3 *b, vec3 *v, mat4 modelview, mat4 persp);
    void Drag(int x, int y, mat4 modelview, mat4 persp);
    void Draw(vec3 color, mat4 modelview, mat4 persp);
    void SetVector(vec3 v);
    void SetBase(vec3 b);
    vec3 *base = NULL, *vec = NULL; // pointers to client data
private:
    float arrowScale = 1;
    vec3 color;
    JoyType mode = JoyType::A_None;
    float plane[4] = {0, 0, 0, 0};
    bool fwdFace = true;
    bool  hit = false;
};

// Arcball: quaternion rotation of reference frame with trackball-like UI

class Arcball {
public:
    Arcball();
    Arcball(mat4 *m, vec2 center, float radius);
    void Set(mat4 *m, vec2 center, float radius);
        // center and radius in pixels
        // mouse behavior and drawing depend solely on center and radius
    void SetCenter(vec2 center);
    void SetCenter(vec2 center, float radius);
    bool Hit(int x, int y);
    void Down(int x, int y);
    void Drag(int x, int y);
    void Up();
    void Wheel(float direction, bool shift);
    void Draw();                // should be in screen mode
    mat4 *GetMatrix();
    Quaternion GetQ();
    vec2 center;                // display center (should be private)
    float radius, scale;        // display radius, scale of m on input (should be private)
private:
    mat4 *m;                    // matrix to adjust (upper 3x3 only)
    Quaternion qstart;          // quaternion representing m on mouse down
    Quaternion qq;              // qstart multiplied by mouse drag
    vec2 mouseDown, mouseMove;  // ball hits on mouse down/drag, in pixels
    vec3 BallV(vec2 mouse);     // vector from origin to mouse ballpick
friend class Framer;
};

// Framer: combination mover and arcball for reference frame adjustment

class Framer {
public:
    Framer();
    Framer(mat4 *m, float radius, mat4 fullview);
    void Set(mat4 *m, float radius, mat4 fullview);
    bool Hit(int x, int y);
    void Down(int x, int y, mat4 modelview, mat4 persp);
    void Drag(int x, int y, mat4 modelview, mat4 persp);
    void Up();
    void Wheel(bool forward, bool shift);
    void Draw(mat4 fullview);
    mat4 *GetMatrix();
private:
    Mover mover;
    Arcball arcball;
    bool moverPicked;
    vec3 base;                  // base of arcball.m on input
};

// Toggle Button

class Toggler {
private:
    bool *on;
    std::string name;
    int x, y;
    float dia;
    vec3 onCol, offCol, ringCol;
public:
    Toggler(bool *on, const char *name, int x, int y, float dia = 12, vec3 onCol = vec3(1,0,0), vec3 offCol = vec3(.7f), vec3 ringCol = vec3(.1f));
    bool On();
    void Draw();
    bool Hit(int xMouse, int yMouse, int proximity = 12);
    bool UpHit(int xMouse, int yMouse, int state, int proximity = 12);
    const char *Name();
};

// Magnifier

class Magnifier {
public:
    int2 srcLoc, srcLocSave, mouseDown, displaySize;
    int blockSize;
    Magnifier(int2 srcLoc = int2(), int2 displaySize = int2(), int blockSize = 20);
    void Down(int x, int y);
    void Drag(int x, int y);
    bool Hit(int x, int y);
    void Display(int2 displayLoc, bool showSrcWindow = true);
};

#endif
