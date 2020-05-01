// CameraArcball.cpp, Copyright (c) Jules Bloomenthal, Seattle, 2018, All rights reserved.

#include "CameraArcball.h"
#include <stdio.h>

// ? Do not pass the window size to glViewport or other pixel-based OpenGL calls.
// ? The window size is in screen coordinates, not pixels.
// ? Use the framebuffer size, which is in pixels, for pixel-based calls.

void CameraAB::Save(const char *filename) {
    FILE *out = fopen(filename, "wb");
    fwrite(&rot, sizeof(mat4), 1, out);
    fwrite(&tran, sizeof(vec4), 1, out);
    fwrite(&fov, sizeof(float), 1, out);
    fwrite(&nearDist, sizeof(float), 1, out);
    fwrite(&farDist, sizeof(float), 1, out);
    fclose(out);
}

bool CameraAB::Read(const char *filename) {
    FILE *in = fopen(filename, "rb");
    if (!in)
        return false;
    bool ok =
        fread(&rot, sizeof(mat4), 1, in) == 1 &&
        fread(&tran, sizeof(vec4), 1, in) == 1 &&
        fread(&fov, sizeof(float), 1, in) == 1 &&
        fread(&nearDist, sizeof(float), 1, in) == 1 &&
        fread(&farDist, sizeof(float), 1, in) == 1;
    fclose(in);
    if (ok) {
        persp = Perspective(fov, aspectRatio, nearDist, farDist);
        modelview = Translate(tran)*rot;
        fullview = persp*modelview;
    }
    return ok;
}

void CameraAB::Set(int *vp) { Set(vp[0], vp[1], vp[2], vp[3]); }

void CameraAB::Set(int scrnX, int scrnY, int scrnW, int scrnH) {
    aspectRatio = (float) scrnW / scrnH;
    persp = Perspective(fov, aspectRatio, nearDist, farDist);
    modelview = Translate(tran)*rot;
    fullview = persp*modelview;
    float minS = (float) (scrnW < scrnH? scrnW : scrnH);
    arcball.Set(&this->rot, vec2((float) (scrnX+scrnW/2), (float) (scrnY+scrnH/2)), minS/2-50);
};

void CameraAB::Set(int scrnX, int scrnY, int scrnW, int scrnH, mat4 rot, vec3 tran, float fov, float nearDist, float farDist, bool invVrt) {
    this->aspectRatio = (float) scrnW / scrnH;
    this->rot = rot;
    this->tran = tran;
    this->fov = fov;
    this->nearDist = nearDist;
    this->farDist = farDist;
    this->invertVertical = invVrt;
    tranSpeed = .01f;
    persp = Perspective(fov, aspectRatio, nearDist, farDist);
    modelview = Translate(tran)*rot;
    fullview = persp*modelview;
    float minS = (float) (scrnW < scrnH? scrnW : scrnH);
    arcball.Set(&this->rot, vec2((float) (scrnX+scrnW/2), (float) (scrnY+scrnH/2)), minS/2-50);
};

void CameraAB::Set(int *vp, mat4 rot, vec3 tran, float fov, float nearDist, float farDist, bool invVrt) {
    Set(vp[0], vp[1], vp[2], vp[3], rot, tran, fov, nearDist, invVrt);
}

void CameraAB::Set(int scrnX, int scrnY, int scrnW, int scrnH, Quaternion qrot, vec3 tran, float fov, float nearDist, float farDist, bool invVrt) {
    rot = qrot.GetMatrix();
    Set(scrnX, scrnY, scrnW, scrnH, rot, tran, fov, nearDist, farDist, invVrt);
}

CameraAB::CameraAB(int *viewport, vec3 eulerAngs, vec3 tran, float fov, float nearDist, float farDist, bool invVrt) {
    rot = RotateX(eulerAngs[0])*RotateY(eulerAngs[1])*RotateZ(eulerAngs[2]);
    Set(viewport, rot, tran, fov, nearDist, farDist, invVrt);
};

CameraAB::CameraAB(int scrnX, int scrnY, int scrnW, int scrnH, vec3 eulerAngs, vec3 tran, float fov, float nearDist, float farDist, bool invVrt) {
    rot = RotateX(eulerAngs[0])*RotateY(eulerAngs[1])*RotateZ(eulerAngs[2]);
    Set(scrnX, scrnY, scrnW, scrnH, rot, tran, fov, nearDist, farDist, invVrt);
};

CameraAB::CameraAB(int scrnX, int scrnY, int scrnW, int scrnH, Quaternion qrot, vec3 tran, float fov, float nearDist, float farDist, bool invVrt) {
    rot = qrot.GetMatrix();
    Set(scrnX, scrnY, scrnW, scrnH, rot, tran, fov, nearDist, farDist, invVrt);
};

void CameraAB::SetModelview(mat4 mv) {
    tranOld = tran = vec3(mv[0][3], mv[1][3], mv[2][3]);    // FrameBase(mv);
    rot = modelview = mv;
    rot[0][3] = rot[1][3] = rot[2][3] = 0;                  // remove tran from rot
    fullview = persp*modelview;
}

void CameraAB::SetFOV(float newFOV) {
    fov = newFOV;
    persp = Perspective(fov, aspectRatio, nearDist, farDist);
    fullview = persp*modelview;
}

float CameraAB::GetFOV() { return fov; }

void CameraAB::Resize(int width, int height) {
    aspectRatio = (float) width/height;
    persp = Perspective(fov, aspectRatio, nearDist, farDist);
    fullview = persp*modelview;
    arcball.Set(&rot, vec2((float) width, (float) height)/2, (float) (width < height? width : height)/2-50);
}

void CameraAB::SetSpeed(float tranS) { tranSpeed = tranS; }

mat4 CameraAB::GetRotate() {
    mat4 moveToCenter = Translate(-rotateCenter);
    mat4 moveBack = Translate(rotateCenter+rotateOffset);
    return moveBack*rot*moveToCenter;
}

void CameraAB::SetRotateCenter(vec3 r) {
    mat4 m = GetRotate();
    vec4 rXformedWithOldRotateCenter = m*r;
    rotateCenter = r;
    m = GetRotate(); // no, this is not redundant!
    vec4 rXformedWithNewRotateCenter = m*r;
    for (int i = 0; i < 3; i++)
        rotateOffset[i] += rXformedWithOldRotateCenter[i]-rXformedWithNewRotateCenter[i];
}

void CameraAB::MouseDown(double x, double y) {
	MouseDown((int) x, (int) y);
}

void CameraAB::MouseDown(int x, int y) {
    arcball.Down(x, y);
    mouseDown = vec2((float) x, (float) y);
    tranOld = tran;
}

void CameraAB::MouseDrag(double x, double y, bool shift) {
	MouseDrag((int) x, (int) y, shift);
}

void CameraAB::MouseDrag(int x, int y, bool shift) {
    vec2 mouse((float) x, (float) y), dif = mouse-mouseDown;
    dif.y = invertVertical? -dif.y : dif.y;
    if (shift)
        tran = tranOld+tranSpeed*vec3(dif.x, -dif.y, 0);
    else
        arcball.Drag(x, y);
    modelview = Translate(tran)*GetRotate();
    fullview = persp*modelview;
}

void CameraAB::MouseWheel(bool forward, bool shift) {
//  if (shift)
        tranOld.z = (tran.z += forward? -.1f : .1f);  // dolly in/out
//  else
//      arcball.Wheel(direction, shift); // **** causes mouse-down jump
    modelview = Translate(tran)*GetRotate();
    fullview = persp*modelview;
}

void CameraAB::MouseUp() {
    tranOld = tran;
    arcball.Up();
}

vec3 EulerFromMatrix(mat4 R) {
    float sy = sqrt(R[0][0]*R[0][0]+R[1][0]*R[1][0]);
    bool singular = sy < 1e-6;
    float x, y, z;
    if (!singular) {
        x = atan2(R[2][1], R[2][2]);
        y = atan2(-R[2][0], sy);
        z = atan2(R[1][0], R[0][0]);
    }
    else {
        x = atan2(-R[1][2], R[1][1]);
        y = atan2(-R[2][0], sy);
        z = 0;
    }
    return vec3(x, y, z);
}

vec3 CameraAB::GetRot() {
    return EulerFromMatrix(*arcball.GetMatrix());
}

vec3 CameraAB::GetTran() {
    return tran;
}

static const char *usage = "\
    mouse-drag:\trotate x,y\n\
    with shift:\ttranslate x,y\n\
    mouse-wheel:\trotate z\n\
    with shift:\ttranslate z";

char *CameraAB::Usage() { return (char *) usage; }
