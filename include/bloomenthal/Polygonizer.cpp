// Polygonizer.cpp (c) Jules Bloomenthal, 2014-18

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <stdio.h>
#include <vector>
#include "Polygonizer.h"
#include "VecMat.h"

namespace {

const int RES = 9;              // # converge iterations
enum {L=0, R, B, T, N, F};      // left, right, bottom, top, near, far
enum {LBN=0, LBF, LTN, LTF, RBN, RBF, RTN, RTF};

// the LBN corner of cube (i, j, k), corresponds with location (i*size, j*size, k*size)
// *** no longer: (start.x+(i-.5)*size, start.y+(j-.5)*size, start.z+(k-.5)*size)

const int HASHBIT = 5;
const int HASHSIZE = ((size_t)(1<<(3*HASHBIT)));
const int MASK = ((1<<HASHBIT)-1);

inline int HASH( int i, int j,int k) {
    return (((( (i&MASK) << HASHBIT) | (j&MASK)) << HASHBIT) | (k&MASK));
}

inline int BIT(int i, int bit) {
    return (i>>bit)&1;
}

inline float RAND() {
    return (rand()&32767)/32767.0f;
}

inline int FLIP(int i, int bit) {
    return i^1<<bit;
}

struct TEST {                   // test the function for a signed value
    vec3 p;                     // location of test
    float value;                // function value at p
    int ok;                     // if value is of correct sign
};

struct CORNER {                 // corner of a cube
    int i, j, k;                // (i, j, k) is index within lattice
    float x, y, z, value;       // location and function value
};

struct CUBE {                   // partitioning cell (cube)
    int i, j, k;                // lattice location of cube
    float values[8];
    bool transects;
};

typedef struct cubes {
    // linked list of cubes acting as stack
    CUBE cube;                  // a single cube
    struct cubes *next;         // remaining elements
} CUBES;

typedef struct centerlist {
    // list of cube locations
    int i, j, k;                // cube location
    struct centerlist *next;    // remaining elements
} CENTERLIST;

typedef struct cornerlist {
    // list of corners
    int i, j, k;                // corner id
    float value;                // corner value
    struct cornerlist *next;    // remaining elements
} CORNERLIST;

typedef struct edgelist {
    // list of edges
    unsigned int i1, j1, k1, i2, j2, k2; // edge corner ids
    int vid;                    // vertex id
    struct edgelist *next;      // remaining elements
} EDGELIST;

// mycalloc used for CUBES *cubes, CENTERLIST **centers, CORNERLIST **corners, EDGELIST **edges
// FreeAll frees edges, corners, centers (cubes deallocated during March()

char *mycalloc (int nitems, int nbytes) {
    char *ptr = (char *) calloc(nitems, nbytes);
    if (ptr == NULL)
        throw("can't allocate memory");
    return ptr;
}

class Process {
public:
    // parameters, function, storage
    ImplicitProc  iProc;
    VertexProc    vProc;
    TriangleProc  tProc;
    float         size;         // cube size
    float         delta;        // normal delta
    int           bounds;       // cube range within lattice
    CUBES        *cubes;        // active cubes
    CENTERLIST  **centers;      // cube center hash table (prevent cycling)
    CORNERLIST  **corners;      // corner value hash table
    EDGELIST    **edges;        // edge and vertex id hash table

    void FreeAll () {
        int index;
        CORNERLIST *l, *lnext;
        CENTERLIST *cl, *clnext;
        EDGELIST *edge, *edgenext;
        if (corners)
            for (index = 0; index < HASHSIZE; index++)
                for (l = corners[index]; l; l = lnext) {
                    lnext = l->next;
                    free(l);    // free CORNERLIST
                }
        if (centers)
            for (index = 0; index < HASHSIZE; index++)
                for (cl = centers[index]; cl; cl = clnext) {
                    clnext = cl->next;
                    free(cl);   // free CENTERLIST
                }
        if (edges)
            for (index = 0; index < 2*HASHSIZE; index++)
                for (edge = edges[index]; edge; edge = edgenext) {
                    edgenext = edge->next;
                    free(edge); // free EDGELIST
                }
        free(edges);            // free array of EDGELIST pointers
        free(corners);          // free array of CORNERLIST pointers
        free(centers);          // free array of CENTERLIST pointers
        edges = NULL;
        corners = NULL;
        centers = NULL;
    }

    TEST Find(int sign, vec3 &p) {
        // search for point with value of given sign (0: neg, 1: pos)
        int i;
        TEST test;
        float range = size;
        test.ok = 1;
        for (i = 0; i < 10000; i++) {
            test.p.x = p.x+range*(RAND()-0.5f);
            test.p.y = p.y+range*(RAND()-0.5f);
            test.p.z = p.z+range*(RAND()-0.5f);
            test.value = iProc(test.p);
            if (sign == (test.value > 0.0))
                return test;
            range = range*1.0005f; // slowly expand search outwards
        }
        test.ok = 0;
        return test;
    }

    CUBE MakeCube(int3 ijk) {
        int npos = 0, nneg = 0;
        CUBE c;
        c.i = ijk.i1;
        c.j = ijk.i2;
        c.k = ijk.i3;
        for (int n = 0; n < 8; n++) {
            c.values[n] = SetCorner(c.i+BIT(n,2), c.j+BIT(n,1), c.k+BIT(n,0));
            float val = c.values[n];
            val < 0? nneg++ : val > 0? npos++ : 0;
        }
        c.transects = nneg > 0 && npos > 0;
        return c;
    }

    void AddToStack(CUBE &c) {
        if (!SetCenter(centers, c.i, c.j, c.k)) {         // not previously set
            // add new cube to top of stack
            CUBES *oldcubes = cubes;
            cubes = (CUBES *) mycalloc(1, sizeof(CUBES)); // freed in March
            cubes->cube = c;
            cubes->next = oldcubes;
        }
    }

    bool AddToStack(vec3 &q) {
        TEST in, out;
        in = Find(1, q);
        out = Find(0, q);
        if (!in.ok || !out.ok) {
            // throw("can't find starting point");
            return false;
        }
        vec3 p;
        Converge(in.p, out.p, in.value, p);
        int3 ijk = LatticeIJK(p);
        CUBE c = MakeCube(ijk);
        if (c.transects)
            AddToStack(c);
        else {
            // no transection (q must graze cube face, edge, or vertex), add face neighbors
            int3 ids[] = {int3(c.i-1, c.j, c.k), int3(c.i+1, c.j, c.k),
                          int3(c.i, c.j-1, c.k), int3(c.i, c.j+1, c.k),
                          int3(c.i, c.j, c.k-1), int3(c.i, c.j, c.k+1)};
            for (int i = 0; i < 6; i++) {
                CUBE c = MakeCube(ids[i]);
                AddToStack(c);
              }
        }
        return true;
    }

    int3 LatticeIJK(vec3 &p) {
        return int3((int) floor(p.x/size), (int) floor(p.y/size), (int) floor(p.z/size));
    }

    void March() {
        bool noabort;
        while (cubes != NULL) {
            // process active cubes till none left
            CUBES *temp = cubes;
            CUBE c = cubes->cube;
            noabort =
                // decompose into tetrahedra and polygonize
                DoTet(&c, LBN, LTN, RBN, LBF) &&
                DoTet(&c, RTN, LTN, LBF, RBN) &&
                DoTet(&c, RTN, LTN, LTF, LBF) &&
                DoTet(&c, RTN, RBN, LBF, RBF) &&
                DoTet(&c, RTN, LBF, LTF, RBF) &&
                DoTet(&c, RTN, LTF, RTF, RBF);
            if (!noabort)
                throw("aborted");
            // pop current cube from stack
            cubes = cubes->next;
            free((char *) temp);
            // test six face directions, maybe add to stack
            TestFace(c.i-1, c.j, c.k, &c, L, LBN, LBF, LTN, LTF);
            TestFace(c.i+1, c.j, c.k, &c, R, RBN, RBF, RTN, RTF);
            TestFace(c.i, c.j-1, c.k, &c, B, LBN, LBF, RBN, RBF);
            TestFace(c.i, c.j+1, c.k, &c, T, LTN, LTF, RTN, RTF);
            TestFace(c.i, c.j, c.k-1, &c, N, LBN, LTN, RBN, RTN);
            TestFace(c.i, c.j, c.k+1, &c, F, LBF, LTF, RBF, RTF);
        }
    } // end March

    bool SetCenter(CENTERLIST *table[], int i, int j, int k)    {
        // set (i,j,k) entry of table[]
        // return true if already set; otherwise, set and return false
        int index = HASH(i, j, k);
        CENTERLIST *newCL, *l, *q = table[index];
        for (l = q; l != NULL; l = l->next)
        if (l->i == i && l->j == j && l->k == k)
            return true;
        newCL = (CENTERLIST *) mycalloc(1, sizeof(CENTERLIST)); // freed in FreeAll
        newCL->i = i;
        newCL->j = j;
        newCL->k = k;
        newCL->next = q;
        table[index] = newCL;
        return false;
    }

    void TestFace (int i, int j, int k, CUBE *old, int face, int c1, int c2, int c3, int c4) {
        // given cube at lattice (i, j, k), and four corners of face,
        // if surface crosses face, compute other four corners of adjacent cube
        // and add new cube to cube stack
        static int facebit[6] = {2, 2, 1, 1, 0, 0};
        int pos = old->values[c1] > 0.0 ? 1 : 0, bit = facebit[face];
        // test if no surface crossing, cube out of bounds, or already visited
        if ((old->values[c2] > 0) == pos &&
            (old->values[c3] > 0) == pos &&
            (old->values[c4] > 0) == pos)
            return;
        if (abs(i) > bounds || abs(j) > bounds || abs(k) > bounds)
            return;
        if (SetCenter(centers, i, j, k))
            return;
        // create new_obj cube
        CUBE c;
        c.i = i;
        c.j = j;
        c.k = k;
        int cids[] = {c1, c2, c3, c4};
        for (int n = 0; n < 4; n++) {
            int cid = cids[n];
            c.values[FLIP(cid, bit)] = old->values[cid];
            c.values[cid] = SetCorner(i+BIT(cid,2), j+BIT(cid,1), k+BIT(cid,0));
        }
        // add new cube to top of stack
        CUBES *oldcubes = cubes;
        cubes = (CUBES *) mycalloc(1, sizeof(CUBES)); // freed in March
        cubes->cube = c;
        cubes->next = oldcubes;
    }

    float SetCorner (int i, int j, int k) {
        // return corner with the given lattice location
        // set (and cache) its function value; for speed, do corner value caching here
        int index = HASH(i, j, k);
        CORNERLIST *l = corners[index];
        for (; l != NULL; l = l->next)
            if (l->i == i && l->j == j && l->k == k)
                return l->value;
        l = (CORNERLIST *) mycalloc(1, sizeof(CORNERLIST)); // freed in FreeAll
        l->i = i; l->j = j; l->k = k;
        l->value = iProc(vec3((float)i*size, (float)j*size, (float)k*size));
        l->next = corners[index];
        corners[index] = l;
        return l->value;
    }

    bool DoTet(CUBE* cube, int c1, int c2, int c3, int c4) {
        // b, c, d should appear clockwise when viewed from a
        // return false if client aborts, true otherwise
        float a = cube->values[c1], b = cube->values[c2], c = cube->values[c3], d = cube->values[c4];
        int index = 0, apos, bpos, cpos, dpos, e1 = 0, e2 = 0, e3 = 0, e4 = 0, e5 = 0, e6 = 0;
        if ((apos = (a > 0.0))) index += 8;
        if ((bpos = (b > 0.0))) index += 4;
        if ((cpos = (c > 0.0))) index += 2;
        if ((dpos = (d > 0.0))) index += 1;
        // index is now 4-bit number representing one of the 16 possible cases
        if (apos != bpos && (e1 = VertId(cube, c1, c2)) < 0)
            return false;
        if (apos != cpos && (e2 = VertId(cube, c1, c3)) < 0)
            return false;
        if (apos != dpos && (e3 = VertId(cube, c1, c4)) < 0)
            return false;
        if (bpos != cpos && (e4 = VertId(cube, c2, c3)) < 0)
            return false;
        if (bpos != dpos && (e5 = VertId(cube, c2, c4)) < 0)
            return false;
        if (cpos != dpos && (e6 = VertId(cube, c3, c4)) < 0)
            return false;
        // 14 productive tetrahedral cases (0000 and 1111 do not yield polygons
        switch (index) {
            case 1:  return tProc(e5, e6, e3);
            case 2:  return tProc(e2, e6, e4);
            case 3:  return tProc(e3, e5, e4) && tProc(e3, e4, e2);
            case 4:  return tProc(e1, e4, e5);
            case 5:  return tProc(e3, e1, e4) && tProc(e3, e4, e6);
            case 6:  return tProc(e1, e2, e6) && tProc(e1, e6, e5);
            case 7:  return tProc(e1, e2, e3);
            case 8:  return tProc(e1, e3, e2);
            case 9:  return tProc(e1, e5, e6) && tProc(e1, e6, e2);
            case 10: return tProc(e1, e3, e6) && tProc(e1, e6, e4);
            case 11: return tProc(e1, e5, e4);
            case 12: return tProc(e3, e2, e4) && tProc(e3, e4, e5);
            case 13: return tProc(e6, e2, e4);
            case 14: return tProc(e5, e3, e6);
        }
        return true;
    } // end dotet

    void SetEdge(EDGELIST *table[], unsigned int i1, unsigned int j1, unsigned int k1, unsigned int i2, unsigned int j2, unsigned int k2, unsigned int vid) {
        // set vertex id for edge
        unsigned int index;
        EDGELIST *newEL;
        if ((i1>i2) || ((i1==i2) && ((j1>j2) || ((j1==j2) && (k1>k2))))) {
            int t;
            t=i1; i1=i2; i2=t;
            t=j1; j1=j2; j2=t;
            t=k1; k1=k2; k2=t;
        }
        index = HASH(i1, j1, k1) + HASH(i2, j2, k2);
        newEL = (EDGELIST *) mycalloc(1, sizeof(EDGELIST)); // freed in FreeAll
        newEL->i1 = i1; newEL->j1 = j1; newEL->k1 = k1;
        newEL->i2 = i2; newEL->j2 = j2; newEL->k2 = k2;
        newEL->vid = vid;
        newEL->next = table[index];
        table[index] = newEL;
    }

    int GetEdge(EDGELIST *table[], unsigned int i1, unsigned int j1, unsigned int k1, unsigned int i2, unsigned int j2, unsigned int k2) {
        // return vertex id for edge; return -1 if not set
        EDGELIST *q;
        if ((i1>i2) || ((i1==i2) && ((j1>j2) || ((j1==j2) && (k1>k2))))) {
            int t;
            t=i1; i1=i2; i2=t;
            t=j1; j1=j2; j2=t;
            t=k1; k1=k2; k2=t;
        };
        q = table[HASH(i1, j1, k1)+HASH(i2, j2, k2)];
        for (; q != NULL; q = q->next)
            if (q->i1 == i1 && q->j1 == j1 && q->k1 == k1 &&
                q->i2 == i2 && q->j2 == j2 && q->k2 == k2)
                    return q->vid;
        return -1;
    }

    int VertId(CUBE *c, int c1, int c2) {
        // vertid: return index for vertex on edge:
        // c1->value and c2->value are presumed of different sign
        // return saved index if any; else compute vertex and save
        int i1 = c->i+BIT(c1,2), j1 = c->j+BIT(c1,1), k1 = c->k+BIT(c1,0);
        int i2 = c->i+BIT(c2,2), j2 = c->j+BIT(c2,1), k2 = c->k+BIT(c2,0);
        int vid = GetEdge(edges, i1, j1, k1, i2, j2, k2);
        if (vid != -1)
            return vid;                          // previously computed
        vec3 a((float)i1*size, (float)j1*size, (float)k1*size), v;
        vec3 b((float)i2*size, (float)j2*size, (float)k2*size), n;
        Converge(a, b, iProc(a), v);             // position
        Normal(v, n, delta);                     // normal
        vid = vProc(v, n);
        SetEdge(edges, i1, j1, k1, i2, j2, k2, vid);
        return vid;
    }

    void Converge(vec3 &p1, vec3 &p2, float v, vec3 &p) {
        int i = 0;
        vec3 pos, neg;
        if (v < 0) {
            pos = p2;
            neg = p1;
        }
        else {
            pos = p1;
            neg = p2;
        }
        while (1) {
            p = 0.5f*(pos+neg);
            if (i++ == RES)
                return;
            if ((iProc(p)) > 0.0)
                 pos = p;
            else neg = p;
        }
    }

    void Normal(vec3 &p, vec3 &n, float delta) {
        float f = iProc(p);
        n.x = iProc(vec3(p.x+delta, p.y, p.z))-f;
        n.y = iProc(vec3(p.x, p.y+delta, p.z))-f;
        n.z = iProc(vec3(p.x, p.y, p.z+delta))-f;
        n = normalize(n);
    }

    Process(ImplicitProc i, VertexProc v, TriangleProc t, float s, float d, int b) :
        iProc(i), vProc(v), tProc(t), size(s), delta(d), bounds(b) {
        // allocate hash tables, freed in FreeAll
        centers = (CENTERLIST **) mycalloc(HASHSIZE, sizeof(CENTERLIST *));
        corners = (CORNERLIST **) mycalloc(HASHSIZE, sizeof(CORNERLIST *));
        edges = (EDGELIST **) mycalloc(2*HASHSIZE, sizeof(EDGELIST *));
        cubes = NULL;
    }

    ~Process() {
        FreeAll();
    }
}; // end Process

} // end namespace

void Polygonize(std::vector<vec3> &starts, float cellSize, int bounds,
                ImplicitProc iProc,
                VertexProc vProc,
                TriangleProc tProc) {
    Process p(iProc, vProc, tProc, cellSize, cellSize/(float)(RES*RES), bounds);
    for (size_t i = 0; i < starts.size(); i++)
        p.AddToStack(starts[i]);
    p.March();
}

void Polygonize(vec3 &start, float cellSize, int bounds,
                ImplicitProc &iProc,
                VertexProc &vProc,
                TriangleProc &tProc) {
    std::vector<vec3> starts(1, start);
    Polygonize(starts, cellSize, bounds,
        iProc,
        vProc,
        tProc);
}
