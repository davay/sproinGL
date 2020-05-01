// CameraArcball.h, Copyright (c) Jules Bloomenthal, 2018, all rights reserved

#ifndef CAMERA_HDR
#define CAMERA_HDR

#include "VecMat.h"
#include "Widgets.h"

// simple camera parameters and methods for mouse
// no-shift
//   drag: rotate about X and Y axes
//   wheel: rotate about Z axis
// shift
//   drag: translate along X and Y axes
//   wheel: translate along Z axis

class CameraAB {
private:
    float   aspectRatio = 1;
    float   fov = 30;
	float   nearDist = .001f, farDist = 500;
    bool    invertVertical = true;             // OpenGL defines origin lower left; Windows defines it upper left
    vec2    mouseDown;                  // for each mouse down, need start point
    vec3    rotateCenter;               // world rotation origin
    vec3    rotateOffset;               // for temp change in world rotation origin
    float   tranSpeed = .01f; //, rotSpeed;
    mat4    rot;                        // rotations controlled by arcball
    vec3    tran, tranOld;              // translation controlled directly by mouse
public:
    Arcball arcball;
    mat4    modelview, persp, fullview; // read-only
    mat4    GetRotate();
    void    SetRotateCenter(vec3 r);
    void    MouseUp();
    void    MouseDown(double x, double y);
    void    MouseDown(int x, int y);
    void    MouseDrag(double x, double y, bool shift);
    void    MouseDrag(int x, int y, bool shift);
    void    MouseWheel(bool forward, bool shift);
    void    Resize(int w, int h);
    float   GetFOV();
    void    SetFOV(float fov);
    void    SetSpeed(float tranSpeed);
    void    SetModelview(mat4 m);
    vec3    GetRot();
    vec3    GetTran();
    char   *Usage();
    // next formerly private:
    void    Set(int *vp);
    void    Set(int scrnX, int scrnY, int scrnW, int scrnH);
    void    Set(int *viewport, mat4 rot, vec3 tran,
             float fov = 30, float nearDist = .001f, float farDist = 500, bool invVrt = true);
    void    Set(int scrnX, int scrnY, int scrnW, int scrnH, mat4 rot, vec3 tran,
             float fov = 30, float nearDist = .001f, float farDist = 500, bool invVrt = true);
    void    Set(int scrnX, int scrnY, int scrnW, int scrnH, Quaternion qrot, vec3 tran,
             float fov = 30, float nearDist = .001f, float farDist = 500, bool invVrt = true);
    void    Save(const char *filename);
    bool    Read(const char *filename);
    CameraAB() { };
    CameraAB(int *vp, vec3 rot = vec3(0,0,0), vec3 tran = vec3(0,0,0),
             float fov = 30, float nearDist = .001f, float farDist = 500, bool invVrt = true);
    CameraAB(int scrnX, int scrnY, int scrnW, int scrnH, vec3 rot = vec3(0,0,0), vec3 tran = vec3(0,0,0),
             float fov = 30, float nearDist = .001f, float farDist = 500, bool invVrt = true);
    CameraAB(int scrnX, int scrnY, int scrnW, int scrnH, Quaternion rot, vec3 tran = vec3(0,0,0),
             float fov = 30, float nearDist = .001f, float farDist = 500, bool invVrt = true);
friend class Arcball;
};

#endif
