// Mesh.h - 3D mesh of triangles

#ifndef MESH_HDR
#define MESH_HDR

#include <vector>
#include "VecMat.h"

using std::vector;

// Read STL Format

struct VertexSTL {
    vec3 point, normal;
    VertexSTL() { }
    VertexSTL(float *p, float *n) : point(vec3(p[0], p[1], p[2])), normal(vec3(n[0], n[1], n[2])) { }
};

int ReadSTL(const char *filename, vector<VertexSTL> &vertices);
    // read vertices from file, three per triangle; return # triangles

// Read OBJ Format

bool ReadAsciiObj(const char    *filename,                  // must be ASCII file
                  vector<vec3>  &points,                    // unique set of points determined by vertex/normal/uv triplets in file
                  vector<int3>  &triangles,                 // array of triangle vertex ids
                  vector<vec3>  *normals  = NULL,           // if non-null, read normals from file, correspond with points
                  vector<vec2>  *textures = NULL,           // if non-null, read uvs from file, correspond with points
                  vector<int>   *triangleGroups = NULL,     // correspond with triangle groups
                  vector<int4>  *quads = NULL);             // optional quadrilaterals
    // set points and triangles; normals, textures, quads optional
    // return true if successful

bool WriteAsciiObj(const char *filename,
                   vector<vec3> &points, vector<vec3> &normals, vector<vec2> &uvs,
                   vector<int3> *triangles = NULL, vector<int4> *quads = NULL);
    // write to file mesh points, normals, and uvs
    // optionally write triangles and/or quadrilaterals

// Normals

void Normalize(vector<vec3> &points, float scale = 1);
    // translate and apply uniform scale so that vertices fit in -scale,+scale in X,Y and 0,1 in Z

void Normalize(vector<VertexSTL> &vertices, float scale = 1);
    // as above

void SetVertexNormals(vector<vec3> &points, vector<int3> &triangles, vector<vec3> &normals);
    // compute/recompute vertex normals as the average of surrounding triangle normals

// Intersection with a Line

struct TriInfo {
    vec4 plane;
    int majorPlane = 0; // 0: XY, 1: XZ, 2: YZ
    vec2 p1, p2, p3;    // vertices projected to majorPlane
    TriInfo() { };
    TriInfo(vec3 p1, vec3 p2, vec3 p3);
};

void BuildTriInfos(vector<vec3> &points, vector<int3> &triangles, vector<TriInfo> &triInfos);
    // for interactive selection

int IntersectWithLine(vec3 p1, vec3 p2, vector<TriInfo> &triInfos, float &alpha);
    // return triangle index of nearest intersected triangle, or -1 if none
    // intersection = p1+alpha*(p2-p1)

#endif
