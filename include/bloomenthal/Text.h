// Text.h: text support

#ifndef TEXT_HDR
#define TEXT_HDR

#include <glad.h>
#include <GLFW/glfw3.h>
#include "GLXtras.h"

class Character {
public:
    GLuint  textureID;  // glyph texture
    int2    gSize;      // glyph size
    int2    bearing;    // offset from baseline to left/top of glyph
    GLuint  advance;    // offset to next glyph
    Character() { textureID = advance = 0; }
    Character(int textureID, int2 gSize, int2 bearing, GLuint advance) :
        textureID(textureID), gSize(gSize), bearing(bearing), advance(advance) { }
};

// character set and current pointer
struct CharacterSet {
    int charRes;
    Character characters[128];
    CharacterSet() { charRes = 0; }
    CharacterSet(const CharacterSet &cs) {
        charRes = 0;
        for (int i = 0; i < 128; i++)
            characters[i] = cs.characters[i];
    }
};

CharacterSet *SetFont(const char *fontName, int charRes = 15, int pixelRes = 15);
    // sets, returns current font

void Text(int x, int y, vec3 color, float scale, const char *format, ...);
    // position null-terminated text at pixel (x, y)

void Text(float x, float y, vec3 color, float scale, const char *format, ...);
    // position null-terminated text at pixel (x, y)

void Text(vec3 p, mat4 m, vec3 color, float scale, const char *format, ...);
    // position text on screen per point p transformed by m

float TextWidth(float scale, const char *format, ...);
    // width in pixels of text displayed with current font and given scale

void RenderText(const char *text, float x, float y, vec3 color, float scale, mat4 view);
    // text with arbitrary orientation

#endif
