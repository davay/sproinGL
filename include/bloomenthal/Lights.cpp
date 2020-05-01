// Lights.cpp - representation/operations on lights

#include "Draw.h"
#include "Lights.h"
#include <stdio.h>

vec3 palette[] = {vec3(1,0,0), vec3(0,1,0), vec3(0,0,1), vec3(1,1,0), vec3(1,0.6f,0), vec3(1,0,1), vec3(0,1,1), vec3(1,1,1)};
int npalette = sizeof(palette)/sizeof(vec3);
static int xCursorOffset = -7, yCursorOffset = -3; // for GLFW

void Lights::Transform(mat4 view, vector<vec3> &xLights) {
    int nlights = lights.size();
    xLights.resize(nlights);
    for (int i = 0; i < nlights; i++) {
        Light l = lights[i];
        vec4 h = view*vec4(l.p, 1);
        xLights[i] = vec3(h.x, h.y, h.z);
    }
}

void Lights::TransformSetColors(mat4 view, vector<vec3> &xLights, vector<vec3> &colors) {
    Transform(view, xLights);
    int nlights = lights.size();
    colors.resize(nlights);
    for (int i = 0; i < nlights; i++) {
        Light l = lights[i];
        colors[i] = palette[l.cid >= npalette? npalette-1 : l.cid];
    }
}

void Lights::Display(void *hover) {
    for (size_t i = 0; i < lights.size(); i++) {
        Light &l = lights[i];
        float dia = hover == &lights[i]? 16.f : 12.f;
        Disk(l.p, dia, palette[l.cid]);
    }
}

Light *Lights::MouseOver(int x, int y, mat4 fullview) {
    for (size_t i = 0; i < lights.size(); i++)
        if (::MouseOver(x, y, lights[i].p, fullview, xCursorOffset, yCursorOffset))
            return &lights[i];
    return NULL;
}

bool Lights::MouseDown(int x, int y, mat4 modelview, mat4 persp) {
    Light *l = MouseOver(x, y, persp*modelview);
    picked = NULL;
    if (l) {
        picked = l;
        mover.Down(&(*l).p, x, y, modelview, persp);
        return true;
    }
    return false;
}

void Lights::MouseDrag(int x, int y, mat4 modelview, mat4 persp) {
    mover.Drag(x, y, modelview, persp);
}

void Lights::AddLight() {
    if (picked) {
        vec3 p = picked->p;
        int cid = picked->cid;
        int nlights = lights.size();
        lights.resize(nlights+1);
        lights[nlights] = Light(p, (cid+1)%npalette);
        picked = &lights[nlights];
    }
}

void Lights::DeleteLight() {
    if (picked) {
        int id = (picked-&lights[0]);
        lights.erase(lights.begin()+id);
        picked = NULL;
    }
}

void Lights::ChangeColor() {
    if (picked)
        picked->cid = (picked->cid+1) % npalette;
}

void Lights::Save(const char *filename) {
    int nlights = lights.size();
    FILE *out = fopen(filename, "wb");
    fwrite(&nlights, sizeof(int), 1, out);
    for (int i = 0; i < nlights; i++)
        fwrite(&lights[i], sizeof(Light), 1, out);
    fclose(out);
}

bool Lights::Read(const char *filename) {
    FILE *in = fopen(filename, "rb");
    if (!in) {
        printf("can't open %s\n", filename);
        return false;
    }
    int nlights;
    if (fread(&nlights, sizeof(int), 1, in) != 1)
        return false;
    lights.resize(nlights);
    for (int i = 0; i < nlights; i++)
        if (fread(&(lights[i].p.x), sizeof(Light), 1, in) != 1)
            return false;
    fclose(in);
    return true;
}
