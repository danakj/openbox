#ifndef __render_texture_h
#define __render_texture_h

#include "render.h"

enum RrTextureType {
    RR_TEXTURE_NONE,
    RR_TEXTURE_TEXT,
    RR_TEXTURE_RGBA
};

struct RrTextureText {
    struct RrFont *font;
    enum RrLayout layout;
    char *string;
};

struct RrTextureRGBA {
    RrData32 *data;
    int x;
    int y;
    int w;
    int h;
};

struct RrTexture;

void RrTextureFreeContents(struct RrTexture *tex);

union RrTextureData {
    struct RrTextureText text;
    struct RrTextureRGBA rgba;
};

struct RrTexture {
    enum RrTextureType type;
    union RrTextureData data;
};

#endif
