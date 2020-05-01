// Polygonizer.h (c) Jules Bloomenthal, 2014-18

#ifndef POLYGONIZER_H
#define POLYGONIZER_H

#include "VecMat.h"
#include <vector>

typedef float (*ImplicitProc)(const vec3 &p);

typedef int (*VertexProc)(const vec3 &p, const vec3 &n);
    // return -1 to abort

typedef bool (*TriangleProc)(int i1, int i2, int i3);
    // return false to abort

void Polygonize(vec3              &start,
                float              cellSize,
                int                bounds,
                ImplicitProc       impFunc,
                VertexProc         vProc,
                TriangleProc       tProc);

void Polygonize(std::vector<vec3> &starts,
                float              cellSize,
                int                bounds,
                ImplicitProc       impFunc,
                VertexProc         vProc,
                TriangleProc       tProc
                );

#endif
