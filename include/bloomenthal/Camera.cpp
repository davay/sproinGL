// Camera.cpp

#include "Camera.h"
#include <stdio.h>

Camera::Camera(float aspectRatio, vec3 rot, vec3 tran, float fov, float nearDist, float farDist, bool invVrt) :
    aspectRatio(aspectRatio), rot(rot), tran(tran), fov(fov),
    nearDist(nearDist), farDist(farDist), invertVertical(invVrt) {
        persp = Perspective(fov, aspectRatio, nearDist, farDist);
        modelview = Translate(tran)*GetRotate();
        fullview = persp*modelview;
};

Camera::Camera(int scrnW, int scrnH, vec3 rot, vec3 tran, float fov, float nearDist, float farDist, bool invVrt) :
    aspectRatio((float) scrnW/scrnH), rot(rot), tran(tran), fov(fov),
    nearDist(nearDist), farDist(farDist), invertVertical(invVrt) {
        persp = Perspective(fov, aspectRatio, nearDist, farDist);
        modelview = Translate(tran)*GetRotate();
        fullview = persp*modelview;
};

void Camera::SetFOV(float newFOV) {
    fov = newFOV;
    persp = Perspective(fov, aspectRatio, nearDist, farDist);
    fullview = persp*modelview;
}

float Camera::GetFOV() {
    return fov;
}

void Camera::Resize(int screenW, int screenH) {
    aspectRatio = (float)screenW/screenH;
    persp = Perspective(fov, aspectRatio, nearDist, farDist);
    fullview = persp*modelview;
}

void Camera::SetSpeed(float rotS, float tranS) {
    rotSpeed = rotS;
    tranSpeed = tranS;
}

mat4 Camera::GetRotate() {
    mat4 moveToCenter = Translate(-rotateCenter);
    mat4 moveBack = Translate(rotateCenter+rotateOffset);
    mat4 rx = RotateX(rot[0]);
    mat4 ry = RotateY(rot[1]);
    mat4 rz = RotateZ(rot[2]);
    return moveBack*rx*ry*rz*moveToCenter;
}

void Camera::SetRotateCenter(vec3 r) {
    mat4 m = GetRotate();
    vec4 rXformedWithOldRotateCenter = m*r;
    rotateCenter = r;
    m = GetRotate(); // no, this is not redundant!
    vec4 rXformedWithNewRotateCenter = m*r;
    for (int i = 0; i < 3; i++)
        rotateOffset[i] += rXformedWithOldRotateCenter[i]-rXformedWithNewRotateCenter[i];
}

void Camera::MouseDrag(int x, int y, bool shift) {
	MouseDrag((double) x, (double) y, shift);
}

void Camera::MouseDrag(double x, double y, bool shift) {
    vec2 mouse((float) x, (float) y), dif = mouse-mouseDown;
    if (shift)
        tran = tranOld+tranSpeed*vec3(dif.x, invertVertical? -dif.y : dif.y, 0);
            // invert y for upward-increasing screen space
    else {
        rot.x = rotOld.x+rotSpeed*dif.y;
        rot.y = rotOld.y+rotSpeed*dif.x;
    }
    modelview = Translate(tran)*GetRotate();
    fullview = persp*modelview;
}

void Camera::MouseWheel(bool forward, bool shift) {
    if (shift)
        tranOld.z = (tran.z += forward? -.1f : .1f);  // dolly in/out
    else
        rotOld.z = (rot.z += 5*(forward? rotSpeed : -rotSpeed));
    modelview = Translate(tran)*GetRotate();
    fullview = persp*modelview;
}

void Camera::MouseUp() {
    rotOld = rot;
    tranOld = tran;
}

void Camera::MouseDown(int x, int y) {
	MouseDown((double) x, (double) y);
}

void Camera::MouseDown(double x, double y) {
    mouseDown = vec2((float) x, (float) y);
    rotOld = rot;
    tranOld = tran;
}

vec3 Camera::GetRot() {
    return rot;
}

vec3 Camera::GetTran() {
    return tran;
}

static const char *usage = "\
\tmouse-drag:\trotate x,y\n\
\twith shift:\ttranslate x,y\n\
\tmouse-wheel:\trotate z\n\
\twith shift:\ttranslate z";

char *Camera::Usage() {
    return (char *) usage;
}
