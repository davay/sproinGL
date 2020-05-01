// Slider.h, copyright (c) Jules Bloomenthal, 2019, all rights reserved

#ifndef SLIDER_HDR
#define SLIDER_HDR

#include <string>
using std::string;

// Sliders

class Slider {
public:
    bool vertical;
    int winW, winH; // window size
    float x, y;     // lower-left location, in pixels
    float size;     // length, in pixels
    vec3 color;
    float loc;      // slider x-position, in pixels
    float min, max;
    string name;
    Slider();
    Slider(int x, int y, int size, float min, float max, float init, bool vertical = true, const char *name = NULL, vec3 *color = NULL);
    void Init(int x, int y, int size, float min, float max, float init, bool vertical = true, const char *name = NULL, vec3 *color = NULL);
    void SetValue(float val);
    void SetRange(float min, float max, float init);
    void Draw(const char *nameOverride = NULL, vec3 *sliderColor = NULL);
    float GetValue();
    bool Mouse(float x, float y);
    bool Mouse(int x, int y);
        // called upon mouse-down or mouse-drag; return true if call results in new slider location
    bool Hit(int x, int y);
    float YFromVal(float val);
};

#endif
