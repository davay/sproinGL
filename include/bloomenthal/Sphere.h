//    Sphere.h
//    Copyright (c) Jules Bloomenthal, 2014-2016
//    All rights reserved

#ifndef SPHERE_HDR
#define SPHERE_HDR

#include <vector>
#include "VecMat.h"

using std::vector;

class SphereVertex {
public:
	vec3 point, normal;	// normals of unit sphere identical to points
	vec2 uv;
	SphereVertex() { }
	SphereVertex(vec3 &p, vec2 &t) : point(p), normal(p), uv(t) { }
	SphereVertex(vec3 &p, vec3 &n, vec2 &t) : point(p), normal(n), uv(t) { }
	SphereVertex(vec3 p, vec3 n, vec2 t) : point(p), normal(n), uv(t) { }
};

void UnitSphere(int res, vector<SphereVertex> *vertices);
	// no degeneracy except at poles

void UnitSphere(int res, vector<vec3> &points, vector<vec2> &uvs, vector<int3> &triangles);
	// degeneracy along u=0/1

#endif
