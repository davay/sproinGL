// Quaternion.h - quaternion operations.

#ifndef QUATERNION_HDR
#define QUATERNION_HDR

#include "VecMat.h"

class Quaternion {
public:
    float x = 0, y = 0, z = 0, w = 0;
    Quaternion() { };
    Quaternion(vec3 axis, float radAng);
    Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) { };
    Quaternion(mat3 &rot);
    Quaternion(mat4 m);
    Quaternion(const Quaternion &a) { x = a.x; y = a.y; z = a.z; w = a.w; }
    Quaternion& operator = (const Quaternion &a) { x = a.x; y = a.y; z = a.z; w = a.w; return *this; }
    Quaternion operator + (const Quaternion &q) const { return Quaternion(x+q.x, y+q.y, z+q.z, w+q.w); }
    Quaternion operator * (float s) const { return Quaternion(s*x, s*y, s*z, s*w); }
    Quaternion operator * (const Quaternion &q) const {
        float xx =  x*q.w+y*q.z-z*q.y+w*q.x;
        float yy = -x*q.z+y*q.w+z*q.x+w*q.y;
        float zz =  x*q.y-y*q.x+z*q.w+w*q.z;
        float ww = -x*q.x-y*q.y-z*q.z+w*q.w;
        return Quaternion(xx, yy, zz, ww);
    }
    float Norm() { return x*x+y*y+z*z+w*w; }
    mat3 Get3x3();
    mat4 GetMatrix();
    void Slerp(Quaternion &qu0, Quaternion &qu1, float t);
};

#endif
