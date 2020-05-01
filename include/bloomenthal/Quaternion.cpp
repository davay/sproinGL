// Quaternion.cpp - quaternion rep & op

#include <float.h>
#include "Quaternion.h"

const int X = 0, Y = 1, Z = 2;
#define _PI 3.141592f

Quaternion::Quaternion(vec3 axis, float radAng) {
    float c = cos(radAng/2), s = sin(radAng/2);
    vec3 a = normalize(axis);
    x = s*a.x; y = s*a.y; z = s*a.z; w = c;
}

Quaternion::Quaternion(mat4 m) {
    mat3 t = mat3(vec3(m[0][0], m[1][0], m[2][0]), vec3(m[0][1], m[1][1], m[2][1]), vec3(m[0][2], m[1][2], m[2][2]));
    *this = Quaternion(t);
}

/* Quaternion::QuaternionOld(mat4 m) {
    // worked for left-handed coord sys, but now use right-handed, so above is transpose
    mat3 t = mat3(vec3(m[0][0], m[0][1], m[0][2]), vec3(m[1][0], m[1][1], m[1][2]), vec3(m[2][0], m[2][1], m[2][2]));
    *this = Quaternion(t);
 } */

Quaternion::Quaternion(mat3 &mat) {
  float s, tr = mat[X][X]+mat[Y][Y]+mat[Z][Z];
  if (tr >= 0) {
    w = (float) sqrt(.25f*(tr+1));
    s = .25f/w;
    x = (mat[Z][Y] - mat[Y][Z]) * s;
    y = (mat[X][Z] - mat[Z][X]) * s;
    z = (mat[Y][X] - mat[X][Y]) * s;
  }
  else {
#define caseMacro(i, j, k, I, J, K)                                       \
    case I:                                                               \
      i = sqrt(.25f*(mat[I][I] - mat[J][J] - mat[K][K] + 1));             \
      s = .25f/i;                                                         \
      j = (mat[I][J] + mat[J][I]) * s;                                    \
      k = (mat[K][I] + mat[I][K]) * s;                                    \
      w = (mat[K][J] - mat[J][K]) * s;                                    \
      break
    switch ((mat[X][X] >= mat[Y][Y])?
           ((mat[X][X] >= mat[Z][Z])? X : Z) :
           ((mat[Y][Y] >= mat[Z][Z])? Y : Z)) {
                caseMacro(x, y, z, X, Y, Z);
                caseMacro(y, z, x, Y, Z, X);
                caseMacro(z, x, y, Z, X, Y);
    }
  }
}

mat3 Quaternion::Get3x3() {
    float norm = Norm();
    if (fabs(norm) < FLT_EPSILON)
        return mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1));
    float s = 2/norm;
    float xs = x*s,  ys = y*s,  zs = z*s;
    float wx = w*xs, wy = w*ys, wz = w*zs;
    float xx = x*xs, xy = x*ys, xz = x*zs;
    float yy = y*ys, yz = y*zs, zz = z*zs;
    return mat3(
        vec3(1 - (yy + zz), xy + wz,       xz - wy),
        vec3(xy - wz,       1 - (xx + zz), yz + wx),
        vec3(xz + wy,       yz - wx,       1 - (xx + yy)));
}

mat4 Quaternion::GetMatrix() {
    mat3 m3 = Get3x3();
    return mat4(m3);
}

float Dot(Quaternion &qL, Quaternion &qR) { return qL.x*qR.x + qL.y*qR.y + qL.z*qR.z + qL.w*qR.w; }

void Quaternion::Slerp(Quaternion& qu0, Quaternion& qu1, float t) {
  float qu0Part, qu1Part, epsilon = .00001f;
  float cosOmega = Dot(qu0, qu1);
  if ((1 + cosOmega) > epsilon) { // usual case
    if ((1 - cosOmega) > epsilon) { // usual case
      float omega = acos(cosOmega);
      float sinOmega = sin(omega);
      qu0Part = sin((1 - t)*omega)/sinOmega;
      qu1Part = sin(t*omega)/sinOmega;
    }
    else { // ends very close
      qu0Part = 1.0f - t;
      qu1Part = t;
    }
    Quaternion s1 = qu0*qu0Part, s2 = qu1*qu1Part; // s1 = Scale(qu0, qu0Part), s2 = Scale(qu1, qu1Part);
    *this = s1+s2; // Add(s1, s2);
  }
  else { // ends nearly opposite
    Quaternion qup(-qu0.y, qu0.x, -qu0.w, qu0.z);
    qu0Part = sin((0.5f - t)*_PI);
    qu1Part = sin(t*_PI);
    Quaternion s1 = qu0*qu0Part, s2 = qup*qu1Part;
    *this = s1+s2;
  }
}

/*
Quaternion Quaternion::Mul(Quaternion &q1, Quaternion &q2) {
    float x =  q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x;
    float y = -q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y;
    float z =  q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z;
    float w = -q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w;
    return Quaternion(x, y, z, w);
}
Quaternion Add(Quaternion &qL, Quaternion &qR) {
  Quaternion qq;
  qq.w = qL.w + qR.w;
  qq.x = qL.x + qR.x;
  qq.y = qL.y + qR.y;
  qq.z = qL.z + qR.z;
  return qq;
}
Quaternion Scale(Quaternion &q, float s) {
  Quaternion qq;
  qq.w = q.w*s;
  qq.x = q.x*s;
  qq.y = q.y*s;
  qq.z = q.z*s;
  return qq;
} */
