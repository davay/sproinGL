// Mesh.cpp - mesh IO and operations

#include "Mesh.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <float.h>
#include <string.h>
#include <cstdlib>

using std::string;
using std::vector;
using std::ios;
using std::ifstream;

// intersections

vec2 MajPln(vec3 &p, int mp) { return mp == 1? vec2(p.y, p.z) : mp == 2? vec2(p.x, p.z) : vec2(p.x, p.y); }

TriInfo::TriInfo(vec3 a, vec3 b, vec3 c) {
    vec3 v1(b-a), v2(c-b), x = normalize(cross(v1, v2));
    plane = vec4(x.x, x.y, x.z, -dot(a, x));
    float ax = fabs(x.x), ay = fabs(x.y), az = fabs(x.z);
    majorPlane = ax > ay? (ax > az? 1 : 3) : (ay > az? 2 : 3);
    p1 = MajPln(a, majorPlane);
    p2 = MajPln(b, majorPlane);
    p3 = MajPln(c, majorPlane);
}

bool LineIntersectPlane(vec3 p1, vec3 p2, vec4 plane, vec3 *intersection, float *alpha) {
  vec3 normal(plane.x, plane.y, plane.z);
  vec3 axis(p2-p1);
  float pdDot = dot(axis, normal);
  if (fabs(pdDot) < FLT_MIN)
      return false;
  float a = (-plane.w-dot(p1, normal))/pdDot;
  if (intersection != NULL)
      *intersection = p1+a*axis;
  if (alpha)
      *alpha = a;
  return true;
}

static bool IsZero(float d) { return d < FLT_EPSILON && d > -FLT_EPSILON; };

int CompareVs(vec2 &v1, vec2 &v2) {
    if ((v1.y > 0 && v2.y > 0) ||           // edge is fully above query point p'
        (v1.y < 0 && v2.y < 0) ||           // edge is fully below p'
        (v1.x < 0 && v2.x < 0))             // edge is fully left of p'
        return 0;                           // can't cross
    float zcross = v2.y*v1.x-v1.y*v2.x;     // right-handed cross-product
    zcross /= length(v1-v2);
    if (IsZero(zcross) && (v1.x <= 0 || v2.x <= 0))
        return 1;                           // on or very close to edge
    if ((v1.y > 0 || v2.y > 0) && ((v1.y-v2.y < 0) != (zcross < 0)))
        return 2;                           // edge is crossed
    else
        return 0;                           // edge not crossed
}

bool IsInside(const vec2 &p, const vec2 &a, const vec2 &b, const vec2 &c) {
    bool odd = false;
    vec2 q = p, v2 = c-q;
    for (int n = 0; n < 3; n++) {
        vec2 v1 = v2;
        v2 = (n==0? a : n==1? b : c)-q;
        if (CompareVs(v1, v2) == 2)
            odd = !odd;
    }
    return odd;
}

void BuildTriInfos(vector<vec3> &points, vector<int3> &triangles, vector<TriInfo> &triInfos) {
    triInfos.resize(triangles.size());
    for (size_t i = 0; i < triangles.size(); i++) {
        int3 &t = triangles[i];
        triInfos[i] = TriInfo(points[t.i1], points[t.i2], points[t.i3]);
    }
}

int IntersectWithLine(vec3 p1, vec3 p2, vector<TriInfo> &triInfos, float &retAlpha) {
    int picked = -1;
    float alpha, minAlpha = FLT_MAX;
    for (size_t i = 0; i < triInfos.size(); i++) {
        TriInfo &t = triInfos[i];
        vec3 inter;
        if (LineIntersectPlane(p1, p2, t.plane, &inter, &alpha)) {
            if (alpha < minAlpha) {
                if (IsInside(MajPln(inter, t.majorPlane), t.p1, t.p2, t.p3)) {
                    minAlpha = alpha;
                    picked = i;
                }
            }
        }
    }
    retAlpha = minAlpha;
    return picked;
}

// center/scale for unit size models

void UpdateMinMax(vec3 p, vec3 &min, vec3 &max) {
    for (int k = 0; k < 3; k++) {
        if (p[k] < min[k]) min[k] = p[k];
        if (p[k] > max[k]) max[k] = p[k];
    }
}

float GetScaleCenter(vec3 &min, vec3 &max, float scale, vec3 &center) {
    center = .5f*(min+max);
    float maxrange = 0;
    for (int k = 0; k < 3; k++)
        if ((max[k]-min[k]) > maxrange)
            maxrange = max[k]-min[k];
    return scale*2.f/maxrange;
}

// normalize STL models

void MinMax(vector<VertexSTL> &points, vec3 &min, vec3 &max) {
    min.x = min.y = min.z = FLT_MAX;
    max.x = max.y = max.z = -FLT_MAX;
    for (int i = 0; i < (int) points.size(); i++)
        UpdateMinMax(points[i].point, min, max);
}

void Normalize(vector<VertexSTL> &vertices, float scale) {
    vec3 min, max, center;
    MinMax(vertices, min, max);
    float s = GetScaleCenter(min, max, scale, center);
    for (int i = 0; i < (int) vertices.size(); i++) {
        vec3 &v = vertices[i].point;
        v = s*(v-center);
    }
}

// normalize vec3 models

void MinMax(vector<vec3> &points, vec3 &min, vec3 &max) {
    min[0] = min[1] = min[2] = FLT_MAX;
    max[0] = max[1] = max[2] = -FLT_MAX;
    for (int i = 0; i < (int) points.size(); i++) {
        vec3 &v = points[i];
        for (int k = 0; k < 3; k++) {
            if (v[k] < min[k]) min[k] = v[k];
            if (v[k] > max[k]) max[k] = v[k];
        }
    }
}

void Normalize(vector<vec3> &points, float scale) {
    vec3 min, max;
    MinMax(points, min, max);
    vec3 center(.5f*(min[0]+max[0]), .5f*(min[1]+max[1]), .5f*(min[2]+max[2]));
    float maxrange = 0;
    for (int k = 0; k < 3; k++)
        if ((max[k]-min[k]) > maxrange)
            maxrange = max[k]-min[k];
    float s = scale*2.f/maxrange;
    for (int i = 0; i < (int) points.size(); i++) {
        vec3 &v = points[i];
        for (int k = 0; k < 3; k++)
            v[k] = s*(v[k]-center[k]);
    }
}

void SetVertexNormals(vector<vec3> &points, vector<int3> &triangles, vector<vec3> &normals) {
    // size normals array and initialize to zero
    int nverts = (int) points.size();
    normals.resize(nverts, vec3(0,0,0));
    // accumulate each triangle normal into its three vertex normals
    for (int i = 0; i < (int) triangles.size(); i++) {
        int3 &t = triangles[i];
        vec3 &p1 = points[t.i1], &p2 = points[t.i2], &p3 = points[t.i3];
        vec3 a(p2-p1), b(p3-p2), n(normalize(cross(a, b)));
        normals[t.i1] += n;
        normals[t.i2] += n;
        normals[t.i3] += n;
    }
    // set to unit length
    for (int i = 0; i < nverts; i++)
        normals[i] = normalize(normals[i]);
}

// ASCII support

bool ReadWord(char* &ptr, char *word, int charLimit) {
    ptr += strspn(ptr, " \t");                  // skip white space
    int nChars = strcspn(ptr, " \t");           // get # non-white-space characters
    if (!nChars)
        return false;                           // no non-space characters
    int nRead = charLimit-1 < nChars? charLimit-1 : nChars;
    strncpy(word, ptr, nRead);
    word[nRead] = 0;                            // strncpy does not null terminate
    ptr += nChars;
        return true;
}

// STL

char *Lower(char *word) {
    for (char *c = word; *c; c++)
        *c = tolower(*c);
    return word;
}

int ReadSTL(const char *filename, vector<VertexSTL> &vertices) {
    // the facet normal should point outwards from the solid object; if this is zero,
    // most software will calculate a normal from the ordered triangle vertices using the right-hand rule
    class Helper {
    public:
        bool status;
        int nTriangles;
        vector<VertexSTL> *verts;
        vector<string> vSpecs;                              // ASCII only
        Helper(const char *filename, vector<VertexSTL> *verts) : verts(verts) {
            char line[1000], word[1000], *ptr = line;
            ifstream inText(filename, ios::in);             // text default mode
            inText.getline(line, 10);
            bool ascii = ReadWord(ptr, word, 10) && !strcmp(Lower(word), "solid");
//          bool ascii = ReadWord(ptr, word, 10) && !_stricmp(word, "solid");
            ascii = false; // hmm!
            if (ascii)
                status = ReadASCII(inText);
            inText.close();
            if (!ascii) {
                FILE *inBinary = fopen(filename, "rb");     // inText.setmode(ios::binary) fails
                if (inBinary) {
                    nTriangles = 0;
                    status = ReadBinary(inBinary);
                    fclose(inBinary);
                }
                else
                    status = false;
            }
        }
        bool ReadASCII(ifstream &in) {
            printf("can't read ASCII STL\n");
            return true;
        }
        bool ReadBinary(FILE *in) {
                  // # bytes      use                  significance
                  // -------      ---                  ------------
                  //      80      header               none
                  //       4      unsigned long int    number of triangles
                  //      12      3 floats             triangle normal
                  //      12      3 floats             x,y,z for vertex 1
                  //      12      3 floats             vertex 2
                  //      12      3 floats             vertex 3
                  //       2      unsigned short int   attribute (0)
                  // endianness is assumed to be little endian
            // in.setmode(ios::binary); doc says setmode good, but compiler says not so
            // sizeof(bool)=1, sizeof(char)=1, sizeof(short)=2, sizeof(int)=4, sizeof(float)=4
            char buf[81];
            int nTriangle = 0;//, vid1, vid2, vid3;
            if (fread(buf, 1, 80, in) != 80) // header
                return false;
            if (fread(&nTriangles, sizeof(int), 1, in) != 1)
                return false;
            while (!feof(in)) {
                vec3 v[3], n;
                if (nTriangle == nTriangles)
                    break;
                if (nTriangles > 5000 && nTriangle && nTriangle%1000 == 0)
                    printf("\rread %i/%i triangles", nTriangle, nTriangles);
                if (fread(&n.x, sizeof(float), 3, in) != 3)
                    printf("\ncan't read triangle %d normal\n", nTriangle);
                for (int k = 0; k < 3; k++)
                    if (fread(&v[k].x, sizeof(float), 3, in) != 3)
                        printf("\ncan't read vid %d\n", verts->size());
                vec3 a(v[1]-v[0]), b(v[2]-v[1]);
                vec3 ntmp = cross(a, b);
                if (dot(ntmp, n) < 0) {
                    vec3 vtmp = v[0];
                    v[0] = v[2];
                    v[2] = vtmp;
                }
                for (int k = 0; k < 3; k++)
                    verts->push_back(VertexSTL((float *) &v[k].x, (float *) &n.x));
                unsigned short attribute;
                if (fread(&attribute, sizeof(short), 1, in) != 1)
                    printf("\ncan't read attribute\n");
                nTriangle++;
            }
            printf("\r\t\t\t\t\t\t\r");
            return true;
        }
    };
    Helper h(filename, &vertices);
    return h.nTriangles;
} // end ReadSTL

// ASCII OBJ

#include <map>
struct Compare {
    bool operator() (const int3 &a, const int3 &b) const {
        return (a.i1==b.i1? (a.i2==b.i2? a.i3 < b.i3 : a.i2 < b.i2) : a.i1 < b.i1);
    }
};

typedef std::map<int3, int, Compare> VidMap;

bool ReadAsciiObj(const char    *filename,
                  vector<vec3>  &points,
                  vector<int3>  &triangles,
                  vector<vec3>  *normals,
                  vector<vec2>  *textures,
                  vector<int>   *triangleGroups,
                  vector<int4>  *quads) {
    // read 'object' file (Alias/Wavefront .obj format); return true if successful;
    // polygons are assumed simple (ie, no holes and not self-intersecting);
    // some file attributes are not supported by this implementation;
    // obj format indexes vertices from 1
    FILE *in = fopen(filename, "r");
    if (!in)
        return false;
    vec2 t;
    vec3 v;
    int group = 0;
    static const int LineLim = 1000, WordLim = 100;
    char line[LineLim], word[WordLim];
    vector<vec3> tmpVertices, tmpNormals;
    vector<vec2> tmpTextures;
    VidMap vidMap;
    for (int lineNum = 0;; lineNum++) {
        line[0] = 0;
        fgets(line, LineLim, in);                  // \ line continuation not supported
        if (feof(in))                              // hit end of file
            break;
        if (strlen(line) >= LineLim-1) {           // getline reads LineLim-1 max
            printf("line %d too long", lineNum);
            return false;
        }
        char *ptr = line;
        if (!ReadWord(ptr, word, WordLim))
            continue;
        Lower(word);
        if (*word == '#')
            continue;
        else if (!strcmp(word, "g"))
            // this implementation: group field significant only if integer
            // .obj format, however, supported arbitrary string identifier
            sscanf(ptr, "%d", &group);
        else if (!strcmp(word, "v")) {           // read vertex coordinates
            if (sscanf(ptr, "%g%g%g", &v.x, &v.y, &v.z) != 3) {
                printf("bad line %d in object file", lineNum);
                return false;
            }
            tmpVertices.push_back(vec3(v.x, v.y, v.z));
        }
        else if (!strcmp(word, "vn")) {          // read vertex normal
            if (sscanf(ptr, "%g%g%g", &v.x, &v.y, &v.z) != 3) {
                printf("bad line %d in object file", lineNum);
                return false;
            }
            tmpNormals.push_back(vec3(v.x, v.y, v.z));
        }
        else if (!strcmp(word, "vt")) {          // read vertex texture
            if (sscanf(ptr, "%g%g", &t.x, &t.y) != 2) {
                printf("bad line in object file");
                return false;
            }
            tmpTextures.push_back(vec2(t.x, t.y));
        }
        else if (!strcmp(word, "f")) {                // read triangle or polygon
            static vector<int> vids;
            vids.resize(0);
            while (ReadWord(ptr, word, WordLim)) {      // read arbitrary # face vid/tid/nid
                // set texture and normal pointers to preceding /
                char *tPtr = strchr(word+1, '/');       // pointer to /, or null if not found
                char *nPtr = tPtr? strchr(tPtr+1, '/') : NULL;
                // use of / is optional (ie, '3' is same as '3/3/3')
                // convert to vid, tid, nid indices (vertex, texture, normal)
                int vid = atoi(word);
                if (!vid) // atoi returns 0 if failure to convert
                    break;
                int tid = tPtr && *++tPtr != '/'? atoi(tPtr) : vid;
                int nid = nPtr && *++nPtr != 0? atoi(nPtr) : vid;
                // standard .obj is indexed from 1, mesh indexes from 0
                vid--;
                tid--;
                nid--;
                if (vid < 0 || tid < 0 || nid < 0) {    // atoi = 0 is conversion failure
                    printf("bad format on line %d\n", lineNum);
                    break;
                }
                int3 key(vid, tid, nid);
                VidMap::iterator it = vidMap.find(key);
                if (it == vidMap.end()) {
                    int nvrts = points.size();
                    vidMap[key] = nvrts;
                    points.push_back(tmpVertices[vid]);
                    if (normals && (int) tmpNormals.size() > nid)
                        normals->push_back(tmpNormals[nid]);
                    if (textures && (int) tmpTextures.size() > tid)
                        textures->push_back(tmpTextures[tid]);
                    vids.push_back(nvrts);
                }
                else
                    vids.push_back(it->second);
            }
            int nids = vids.size();
            if (nids < 3)
                printf("nids = %i!, line %i = %s\n", nids, lineNum, line);
            if (nids == 3) {
                int id1 = vids[0], id2 = vids[1], id3 = vids[2];
                if (normals && (int) normals->size() > id1) {
                    vec3 &p1 = points[id1], &p2 = points[id2], &p3 = points[id3];
                    vec3 a(p2-p1), b(p3-p2), n(cross(a, b));
                    if (dot(n, (*normals)[id1]) < 0) {
                        int tmp = id1;
                        id1 = id3;
                        id3 = tmp;
                    }
                }
                // create triangle
                triangles.push_back(int3(id1, id2, id3));
                if (triangleGroups)
                    triangleGroups->push_back(group);
            }
            else if (nids == 4 && quads)
                quads->push_back(int4(vids[0], vids[1], vids[2], vids[3]));
            else
                // create polygon as nvids-2 triangles
                for (int i = 1; i < nids-1; i++) {
                    triangles.push_back(int3(vids[0], vids[i], vids[(i+1)%nids]));
                    if (triangleGroups)
                        triangleGroups->push_back(group);
                }
        } // end "f"
        else if (*word == 0 || *word == '\n')               // skip blank line
            continue;
        else {                                              // unrecognized attribute
            // printf("unsupported attribute in object file: %s", word);
            continue; // return false;
        }
    } // end read til end of file
    // if (vertexNormals)
    //  SetVertexNormals(vertices, triangles, *vertexNormals);
    return true;
} // end ReadAsciiObj

bool WriteAsciiObj(const char *filename, vector<vec3> &points, vector<vec3> &normals, vector<vec2> &uvs, vector<int3> *triangles, vector<int4> *quads) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("can't write %s\n", filename);
        return false;
    }
    for (size_t i = 0; i < points.size(); i++)
        fprintf(file, "v %f %f %f \n", points[i].x, points[i].y, points[i].z);
    fprintf(file, "\n");
    for (size_t i = 0; i < normals.size(); i++)
        fprintf(file, "vn %f %f %f \n", normals[i].x, normals[i].y, normals[i].z);
    fprintf(file, "\n");
    for (size_t i = 0; i < uvs.size(); i++)
        fprintf(file, "vt %f %f \n", uvs[i].x, uvs[i].y);
    fprintf(file, "\n");
    // write triangles, quads (adding 1 to all vertex indices per OBJ format)
    if (triangles) {
        for (size_t i = 0; i < triangles->size(); i++)
            fprintf(file, "f %d %d %d \n", 1+(*triangles)[i].i1, 1+(*triangles)[i].i2, 1+(*triangles)[i].i3);
        fprintf(file, "\n");
    }
    if (quads)
        for (size_t i = 0; i < quads->size(); i++)
            fprintf(file, "f %d %d %d %d \n", 1+(*quads)[i].i1, 1+(*quads)[i].i2, 1+(*quads)[i].i3, 1+(*quads)[i].i4);
    fclose(file);
    return true;
}
