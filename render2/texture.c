#include "texture.h"
#include "surface.h"
#include <stdlib.h>
#include <string.h>

void RrTextureFreeContents(struct RrTexture *tex)
{
    switch (tex->type) {
    case RR_TEXTURE_NONE:
        break;
    case RR_TEXTURE_TEXT:
        free(tex->data.text.string);
        break;
    case RR_TEXTURE_RGBA:
        break;
    }
    tex->type = RR_TEXTURE_NONE;
}

void RrTextureSetRGBA(struct RrSurface *sur,
                      int texnum,
                      RrData32 *data,
                      int x,
                      int y,
                      int w,
                      int h)
{
    struct RrTexture *tex = RrSurfaceTexture(sur, texnum);

    if (!tex) return;
    RrTextureFreeContents(tex);
    tex->type = RR_TEXTURE_RGBA;
    tex->data.rgba.data = data;
    tex->data.rgba.x = x;
    tex->data.rgba.y = y;
    tex->data.rgba.w = w;
    tex->data.rgba.h = h;
}

void RrTextureSetText(struct RrSurface *sur,
                      int texnum,
                      struct RrFont *font,
                      enum RrLayout layout,
                      const char *text)
{
    struct RrTexture *tex = RrSurfaceTexture(sur, texnum);
    int l;

    if (!tex) return;
    RrTextureFreeContents(tex);
    tex->type = RR_TEXTURE_TEXT;
    tex->data.text.font = font;
    tex->data.text.layout = layout;

    l = strlen(text) + 1;
    tex->data.text.string = malloc(l);
    memcpy(tex->data.text.string, text, l);
}
