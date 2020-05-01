// Lights.h

#ifndef LIGHTS_HDR
#define LIGHTS_HDR

#include "Camera.h"
#include "VecMat.h"
#include "Widgets.h"
#include <vector>

using std::vector;

class Light {
public:
    vec3 p;
    int cid;
    Light(vec3 p = vec3(0,0,0), int cid = 0) : p(p), cid(cid) { }
};

class Lights {
public:
    Lights() { picked = NULL; }
    Lights(vec3 p) { lights.resize(1, p); picked = NULL; }
    int NLights() { return lights.size(); }
    vector<Light> lights;
    Light *picked;
    Mover mover;
    void Transform(mat4 view, vector<vec3> &xLights);
    void TransformSetColors(mat4 view, vector<vec3> &xLights, vector<vec3> &colors);
    void Display(void *hover);
    Light *MouseOver(int x, int y, mat4 fullview);
    bool MouseDown(int x, int y, mat4 modelview, mat4 persp);
    void MouseDrag(int x, int y, mat4 modelview, mat4 persp);
    void ChangeColor();
    void AddLight();
    void DeleteLight();
    void Save(const char *filename);
    bool Read(const char *filename);
};

#endif
