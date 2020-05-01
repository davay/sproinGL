// Color.cpp, Copyright (c) Jules Bloomenthal, Seattle, 2018, All rights reserved.

#include <glad.h>
#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include "Color.h"
#include "Draw.h"

// Color

vec3 RGBfromHSV(vec3 hsv) {
    hsv.x *= 360.f;                       // text expects h to be [0..360] but we use [0..1]
    hsv.x = fmod(hsv.x, 360.f);           // degrees cannot exceed 360
    if (abs(hsv.y) < FLT_MIN)             // color is on the black-and-white center line
        return vec3(hsv.z, hsv.z, hsv.z);
    if (abs(hsv.x-360.f) < FLT_MIN)       // change 360 to 0 degrees
        hsv.x = 0;
    hsv.x = hsv.x/60.f;                   // h now in [0,6]
    int i = (int) floor(hsv.x);           // floor returns the largest integer <= h
    float f = hsv.x-i;
    float p = hsv.z*(1-hsv.y);
    float q = hsv.z*(1-(hsv.y*f));
    float t = hsv.z*(1-(hsv.y*(1-f)));
    switch (i) {
        case 0:  return vec3(hsv.z, t, p);
        case 1:  return vec3(q, hsv.z, p);
        case 2:  return vec3(p, hsv.z, t);
        case 3:  return vec3(p, q, hsv.z);
        case 4:  return vec3(t, p, hsv.z);
        default: return vec3(hsv.z, p, q);
    }
}

vec3 HSVfromRGB(vec3 rgb) {
    class Helper { public:
        float Mx(float a, float b, float c) { return a > b? (a > c? a : c) : (b > c? b : c); }
        float Mn(float a, float b, float c) { return a < b? (a < c? a : c) : (b < c? b : c); }
        bool IsZero(float a) { return a > -FLT_MIN && a < FLT_MIN; }
        vec3 hsv;
        Helper(vec3 rgb) {
            float mx = Mx(rgb[0], rgb[1], rgb[2]);
            float mn = Mn(rgb[0], rgb[1], rgb[2]);
            hsv[2] = mx;
            if (!IsZero(mx))
                hsv[1] = (mx-mn)/mx;
            else
                hsv[1] = 0;
            if (IsZero(hsv[1]))
                hsv[0] = -1;                            // achromatic case, no hue
            else {
                float delta = mx-mn;
                if (IsZero(rgb[0]-mx))
                    hsv[0] = (rgb[1]-rgb[2])/delta;     // resulting color is between yellow and magenta
                else if (IsZero(rgb[1]-mx))
                    hsv[0] = 2+(rgb[2]-rgb[0])/delta;   // resulting color is between cyan and yellow
                else if (IsZero(rgb[2]-mx))
                    hsv[0] = 4+(rgb[0]-rgb[1])/delta;   // resulting color is between magenta and cyan
                hsv[0] = hsv[0]*60;                     // convert hue to degrees
                if (hsv[0] < 0.0)
                    hsv[0] += 360;                      // ensure hue is nonnegative
            }
            hsv[0] /= 360.f;                            // h, s, and v to be in range [0..1]
        }
    } h(rgb);
    return h.hsv;
}

// colors

#include <string.h>
#include <map>
using std::string;

struct NamedColor {
    vec3 color;
    const char *name;
    NamedColor(double r, double g, double b, const char *n) {
        color = vec3((float) r, (float) g, (float) b);
        name = n;
    }
    NamedColor(vec3 c, const char *n) { color = c; name = n; }
};

struct Compare { bool operator() (const string &a, const string &b) const { return a.compare(b) > 0; }};

std::map<string, vec3, Compare> namedColors;

static const NamedColor staticNamedColors[] = {
    NamedColor(0.502, 0.502, 0.502, "Gray"),
    NamedColor(0.000, 0.000, 0.000, "Black"),
    NamedColor(0.000, 0.000, 1.000, "Blue"),
    NamedColor(0.000, 1.000, 1.000, "Cyan"),
    NamedColor(0.000, 0.502, 0.000, "Green"),
    NamedColor(0.000, 1.000, 0.000, "Lime"),
    NamedColor(0.196, 0.804, 0.196, "LimeGreen"),
    NamedColor(1.000, 1.000, 0.000, "Yellow"),
    NamedColor(1.000, 0.000, 1.000, "Magenta"),
    NamedColor(1.000, 0.000, 0.000, "Red"),
    NamedColor(1.000, 1.000, 1.000, "White"),
    NamedColor(0.753, 0.753, 0.753, "Silver"),
    NamedColor(0.663, 0.663, 0.663, "DarkGray"),
    NamedColor(0.412, 0.412, 0.412, "DimGray"),
    NamedColor(0.467, 0.533, 0.600, "LightSlateGray"),
    NamedColor(0.439, 0.502, 0.565, "SlateGray"),
    NamedColor(0.184, 0.310, 0.310, "DarkSlateGray"),
    NamedColor(0.294, 0.000, 0.510, "Indigo"),
    NamedColor(0.282, 0.239, 0.545, "DarkSlateBlue"),
    NamedColor(0.000, 0.000, 0.502, "Navy"),
    NamedColor(0.000, 0.000, 0.545, "DarkBlue"),
    NamedColor(0.098, 0.098, 0.439, "MidnightBlue"),
    NamedColor(0.000, 0.000, 0.804, "MediumBlue"),
    NamedColor(0.255, 0.412, 0.882, "RoyalBlue"),
    NamedColor(0.118, 0.565, 1.000, "DodgerBlue"),
    NamedColor(0.000, 0.749, 1.000, "DeepSkyBlue"),
    NamedColor(0.529, 0.808, 0.980, "LightSkyBlue"),
    NamedColor(0.498, 0.831, 1.000, "BrightSkyBlue"),
    NamedColor(0.529, 0.808, 0.922, "SkyBlue"),
    NamedColor(0.392, 0.584, 0.929, "CornflowerBlue"),
    NamedColor(0.275, 0.510, 0.706, "SteelBlue"),
    NamedColor(0.416, 0.353, 0.804, "SlateBlue"),
    NamedColor(0.482, 0.408, 0.933, "MediumSlateBlue"),
    NamedColor(0.000, 1.000, 1.000, "Aqua"),
    NamedColor(0.000, 0.929, 0.929, "PaleCyan"),
    NamedColor(0.000, 0.808, 0.820, "DarkTurquoise"),
    NamedColor(0.282, 0.820, 0.800, "MediumTurquoise"),
    NamedColor(0.251, 0.878, 0.816, "Turquoise"),
    NamedColor(0.125, 0.698, 0.667, "LightSeaGreen"),
    NamedColor(0.373, 0.620, 0.627, "CadetBlue"),
    NamedColor(0.000, 0.545, 0.545, "DarkCyan"),
    NamedColor(0.000, 0.502, 0.502, "Teal"),
    NamedColor(0.180, 0.545, 0.341, "SeaGreen"),
    NamedColor(0.561, 0.737, 0.545, "DarkSeaGreen"),
    NamedColor(0.000, 0.392, 0.000, "DarkGreen"),
    NamedColor(0.133, 0.545, 0.133, "ForestGreen"),
    NamedColor(0.333, 0.420, 0.184, "DarkOliveGreen"),
    NamedColor(0.420, 0.557, 0.137, "OliveDrab"),
    NamedColor(0.235, 0.702, 0.443, "MediumSeaGreen"),
    NamedColor(0.400, 0.804, 0.667, "MediumAquamarine"),
    NamedColor(0.498, 1.000, 0.831, "Aquamarine"),
    NamedColor(0.000, 0.980, 0.604, "MediumSpringGreen"),
    NamedColor(0.000, 1.000, 0.498, "SpringGreen"),
    NamedColor(0.565, 0.933, 0.565, "LightGreen"),
    NamedColor(0.596, 0.984, 0.596, "PaleGreen"),
    NamedColor(0.486, 0.988, 0.000, "LawnGreen"),
    NamedColor(0.498, 1.000, 0.000, "Chartreuse"),
    NamedColor(0.678, 1.000, 0.184, "GreenYellow"),
    NamedColor(0.604, 0.804, 0.196, "YellowGreen"),
    NamedColor(0.502, 0.502, 0.000, "Olive"),
    NamedColor(0.502, 0.000, 0.000, "Maroon"),
    NamedColor(0.545, 0.000, 0.545, "DarkMagenta"),
    NamedColor(0.502, 0.000, 0.502, "Purple"),
    NamedColor(0.576, 0.439, 0.859, "MediumPurple"),
    NamedColor(0.541, 0.169, 0.886, "BlueViolet"),
    NamedColor(0.580, 0.000, 0.827, "DarkViolet"),
    NamedColor(0.600, 0.196, 0.800, "DarkOrchid"),
    NamedColor(0.729, 0.333, 0.827, "MediumOrchid"),
    NamedColor(0.855, 0.439, 0.839, "Orchid"),
    NamedColor(1.000, 0.000, 1.000, "Fuchsia"),
    NamedColor(0.867, 0.627, 0.867, "Plum"),
    NamedColor(0.780, 0.082, 0.522, "MediumVioletRed"),
    NamedColor(0.859, 0.439, 0.576, "PaleVioletRed"),
    NamedColor(1.000, 0.412, 0.706, "HotPink"),
    NamedColor(1.000, 0.078, 0.576, "DeepPink"),
    NamedColor(0.863, 0.078, 0.235, "Crimson"),
    NamedColor(0.545, 0.000, 0.000, "DarkRed"),
    NamedColor(0.804, 0.361, 0.361, "IndianRed"),
    NamedColor(0.698, 0.133, 0.133, "FireBrick"),
    NamedColor(0.737, 0.561, 0.561, "RosyBrown"),
    NamedColor(0.647, 0.165, 0.165, "Brown"),
    NamedColor(0.545, 0.271, 0.075, "SaddleBrown"),
    NamedColor(0.627, 0.322, 0.176, "Sienna"),
    NamedColor(0.824, 0.412, 0.118, "Chocolate"),
    NamedColor(0.804, 0.522, 0.247, "Peru"),
    NamedColor(0.722, 0.525, 0.043, "DarkGoldenrod"),
    NamedColor(0.741, 0.718, 0.420, "DarkKhaki"),
    NamedColor(0.871, 0.722, 0.529, "Burlywood"),
    NamedColor(0.824, 0.706, 0.549, "Tan"),
    NamedColor(0.855, 0.647, 0.125, "Goldenrod"),
    NamedColor(0.914, 0.588, 0.478, "DarkSalmon"),
    NamedColor(0.933, 0.510, 0.933, "Violet"),
    NamedColor(1.000, 0.271, 0.000, "OrangeRed"),
    NamedColor(1.000, 0.388, 0.278, "Tomato"),
    NamedColor(1.000, 0.498, 0.314, "Coral"),
    NamedColor(0.980, 0.502, 0.447, "Salmon"),
    NamedColor(0.941, 0.502, 0.502, "LightCoral"),
    NamedColor(1.000, 0.627, 0.478, "LightSalmon"),
    NamedColor(0.957, 0.643, 0.376, "SandyBrown"),
    NamedColor(1.000, 0.549, 0.000, "DarkOrange"),
    NamedColor(1.000, 0.647, 0.000, "Orange"),
    NamedColor(1.000, 0.843, 0.000, "Gold"),
    NamedColor(1.000, 0.714, 0.757, "LightPink"),
    NamedColor(1.000, 0.753, 0.796, "Pink"),
    NamedColor(0.980, 0.980, 0.824, "LightGoldenrodYellow"),
    NamedColor(0.933, 0.910, 0.667, "PaleGoldenrod"),
    NamedColor(0.941, 0.902, 0.549, "Khaki"),
    NamedColor(0.961, 0.871, 0.702, "Wheat"),
    NamedColor(1.000, 0.855, 0.725, "PeachPuff"),
    NamedColor(1.000, 0.871, 0.678, "NavajoWhite"),
    NamedColor(1.000, 0.894, 0.710, "Moccasin"),
    NamedColor(1.000, 0.894, 0.769, "Bisque"),
    NamedColor(0.980, 0.922, 0.843, "AntiqueWhite"),
    NamedColor(0.980, 0.941, 0.902, "Linen"),
    NamedColor(0.992, 0.961, 0.902, "OldLace"),
    NamedColor(0.961, 0.961, 0.863, "Beige"),
    NamedColor(0.902, 0.902, 0.980, "Lavender"),
    NamedColor(0.847, 0.749, 0.847, "Thistle"),
    NamedColor(0.863, 0.863, 0.863, "Gainsboro"),
    NamedColor(0.827, 0.827, 0.827, "LightGray"),
    NamedColor(0.678, 0.847, 0.902, "LightBlue"),
    NamedColor(0.686, 0.933, 0.933, "PaleTurquoise"),
    NamedColor(0.690, 0.878, 0.902, "PowderBlue"),
    NamedColor(0.690, 0.769, 0.871, "LightSteelBlue"),
    NamedColor(0.878, 1.000, 1.000, "LightCyan"),
    NamedColor(0.941, 1.000, 1.000, "Azure"),
    NamedColor(0.941, 0.973, 1.000, "AliceBlue"),
    NamedColor(0.941, 1.000, 0.941, "Honeydew"),
    NamedColor(0.961, 1.000, 0.980, "MintCream"),
    NamedColor(0.961, 0.961, 0.961, "WhiteSmoke"),
    NamedColor(1.000, 0.941, 0.961, "LavenderBlush"),
    NamedColor(1.000, 0.961, 0.933, "Seashell"),
    NamedColor(1.000, 0.894, 0.882, "MistyRose"),
    NamedColor(1.000, 0.922, 0.804, "BlanchedAlmond"),
    NamedColor(1.000, 0.937, 0.835, "PapayaWhip"),
    NamedColor(1.000, 0.973, 0.863, "Cornsilk"),
    NamedColor(1.000, 0.980, 0.804, "LemonChiffon"),
    NamedColor(1.000, 1.000, 0.878, "LightYellow"),
    NamedColor(1.000, 1.000, 0.941, "Ivory"),
    NamedColor(1.000, 0.980, 0.941, "FloralWhite"),
    NamedColor(0.973, 0.973, 1.000, "GhostWhite"),
    NamedColor(1.000, 0.980, 0.980, "Snow")
};

bool InitNamedColors() {
    for (size_t i = 0; i < sizeof(staticNamedColors)/sizeof(NamedColor); i++) {
        NamedColor nc = staticNamedColors[i];
        namedColors.insert(std::pair<string, vec3>(string(nc.name), nc.color));
    }
    return true;
}

bool initNamedColors = InitNamedColors();

int NStockColors() { return sizeof(staticNamedColors)/sizeof(NamedColor); }

vec3 GetStockColor(int i) { return staticNamedColors[i].color; }

void Block(int xi, int yi, int wi, int hi, vec3 col) {
    float x = (float) xi, y = (float) yi, w = (float) wi, h = (float) hi;
    vec3 c(x, y, 0);
    Quad(c, c+vec3(w,0,0), c+vec3(w,h,0), c+vec3(0,h,0), true, col);
}

void DrawStockColors(int screenX, int screenY, int blockWidth, int blockHeight, int nColumns, int margin, vec3 *background) {
    int nColors = NStockColors(), nRows = 1+nColors/nColumns;
    UseDrawShader(ScreenMode());
    for (int k = 0; k < nColors; k++) {
        vec3 color = GetStockColor(k);
        int col = k%nColumns, row = nRows-1-k/nColumns;
        if (background)
            Block(screenX+col*(blockWidth+margin), screenY+row*(blockHeight+margin), blockWidth+2*margin, blockHeight+2*margin, *background);
        Block(margin+screenX+col*(blockWidth+margin), margin+screenY+row*(blockHeight+margin), blockWidth, blockHeight, color);
    }
}

