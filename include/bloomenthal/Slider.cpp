// Slider.cpp - Sliders, copyright (c) Jules Bloomenthal, 2019, all rights reserved

#include <glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "Draw.h"
#include "Slider.h"
#include "Text.h"

vec3 blk(0, 0, 0), wht(1);
vec3 offWht(240.f/255.f, 240.f/255.f, 240.f/255.f);
vec3 ltGry(227/255.f), mdGry(160/255.f), dkGry(105/255.f);

// Sliders

Slider::Slider() {
    x = y = size = 0;
    winW = -1;
    Init(0, 0, 80, 0, 1, .5f, true, NULL, NULL);
}

Slider::Slider(int x, int y, int size, float min, float max, float init, bool vertical, const char *nameA, vec3 *col) {
    Init(x, y, size, min, max, init, true, nameA, col);
}

int Round(float f)  { return (int)(f < 0.? ceil(f-.5f) : floor(f+.5f)); }

void Slider::Init(int xi, int yi, int sizei, float min, float max, float init, bool vertical, const char *nameA, vec3 *col) {
        x = static_cast<float>(xi);
        y = static_cast<float>(yi);
        size = static_cast<float>(sizei);
        this->vertical = vertical;
        if (nameA)
            name = string(nameA);
        SetRange(min, max, init);
        color = col? *col : vec3(0,0,0);
        winW = -1;
        float off = vertical? y : x;
        loc = off+(float)(init-min)/((max-min)*size);
}

float Slider::GetValue() {
    float ref = vertical? y : x;
    return min+((float)(loc-ref)/size)*(max-min);
}

void Slider::SetValue(float val) {
    float off = vertical? y : x;
    loc = off+(float)(val-min)/(max-min)*size;
}

void Slider::SetRange(float min, float max, float init) {
    this->min = min;
    this->max = max;
    float off = vertical? y : x;
    loc = off+(float)((init-min)/(max-min))*size;
}

void Rect(float x, float y, float w, float h, vec3 col) {
    Quad(vec3(x, y, 0), vec3(x+w, y, 0), vec3(x+w, y+h, 0), vec3(x, y+h, 0), true, col);
}

void Slider::Draw(const char *nameOverride, vec3 *sCol) {
    float grays[] = {160, 105, 227, 255};
    vec3 knobCol = sCol? *sCol : vec3(0,0,0), slideCol = .8f*knobCol;
    if (vertical) {
        for (int i = 0; i < 4; i++)
            Rect(x-1+i, y, 1, size, vec3(grays[i] / 255.f));
        Rect(x-1, y, 4, 1, wht);
        Rect(x, y+1, 1, 1, ltGry);
        Rect(x-1, y+size-1, 3, 1, mdGry);
        // slider
        Rect(x-10, loc-3, 20, 7, offWht); // whole knob
        Rect(x-10, loc-3, 20, 1, dkGry);  // bottom
        Rect(x+10, loc-3, 1, 7, dkGry);   // right
        Rect(x-10, loc-2, 1, 6, wht);     // left
        Rect(x-10, loc+3, 20, 1, wht);    // top
        Rect(x-9, loc-1, 1, 4, ltGry);    // 1 pixel right of left
        Rect(x-9, loc+2, 18, 1, ltGry);   // 1 pixel below top
        Rect(x-9, loc-2, 18, 1, mdGry);   // 1 pixel above bottom
        Rect(x+9, loc-2, 1, 5, mdGry);    // 1 pixel left of right
    }
    else {
        Line(vec2(x, y-2), vec2(x+size, y-2), 2, slideCol);
        if (sCol)
            Line(vec2(loc, y-11), vec2(loc, y+7), 4, knobCol);
        else
            Line(vec2(loc, y-11), vec2(loc, y+7), 4, vec3(0,0,0));
/*      for (int i = 0; i < 4; i++)
            Rect(x, y-i+1, size, 1, vec3(grays[i]/255.f));
        Rect(x+size-1, y-1, 1, 3, wht);
        Rect(x+size-2, y, 1, 1, ltGry);
        Rect(x, y-1, 1, 3, mdGry);
        // slider
        Rect(iloc-3, y-10, 7, 20, offWht);
        Rect(iloc+3, y-9, 1, 20, dkGry);
        Rect(iloc-3, y-9, 1, 19, wht);
        Rect(iloc-3, y+10, 6, 1, wht);
        Rect(iloc-3, y-10, 7, 1, dkGry);
        Rect(iloc-2, y-9, 1, 18, ltGry);
        Rect(iloc+2, y-9, 1, 19, mdGry);
        Rect(iloc-2, y+9, 4, 1, ltGry);
        Rect(iloc-2, y-9, 5, 1, mdGry); */
    }
    if (nameOverride || !name.empty()) {
        const char *s = nameOverride? nameOverride : name.c_str();
        if (winW == -1)
            GetViewportSize(winW, winH);
        char buf[100], num[100];
        float val = GetValue();
        vec3 col(color[0], color[1], color[2]);
        char *start = val >= 0 && val < 1? buf+1 : buf; // skip leading zero
        if (vertical) {
            sprintf(buf, val >= 1? "%3.2f" : "%3.3f", val);
            float wName = TextWidth(9, s);
            float wBuf = TextWidth(9, start);
            Text(x-wName/2, y+size+6, col, 24, s);
            Text(x+1-wBuf/2, y-17, col, 24, start);
        }
        else {
            sprintf(num, val >= 1 ? "%3.2f" : val < .001 ? "%4.4f" : "%3.3f", val);
            sprintf(buf, "%s:%s", s, val >= 0 && val < 1 ? num+1 : num); // +1: skip zero
            Text(x+size+8, y-8, sCol ? .6f * (*sCol) : blk, 24, buf);
            //  Text(x+size+9, y-8, col, 24, buf);
        }
    }
}

bool Slider::Hit(int xx, int yy) {
    return vertical?
        xx >= x-16 && xx <= x+16 && yy >= y-32 && yy <= y+size+27 :
        xx >= x && xx <= x+size && yy >= y-10 && yy <= y+10;
}

bool Slider::Mouse(float x, float y) {
    // snap to mouse location
    float old = loc;
    float mouse = vertical ? y : x;
    float min = vertical ? y : x;
    loc = mouse < min ? min : mouse > min+size ? min+size : mouse;
    return old != loc;
}

bool Slider::Mouse(int x, int y) {
    return Mouse((float)x, (float)y);
}

float Slider::YFromVal(float val) {
    return y+size*((val-min)/(max-min));
}
