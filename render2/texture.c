#include "instance.h"
#include "texture.h"
#include "surface.h"
#include "font.h"
#include "glft/glft.h"
#include <stdlib.h>
#include <assert.h>
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
        glDeleteTextures(1, &tex->data.rgba.texid);
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
    unsigned int num;
    struct RrTexture *tex = RrSurfaceTexture(sur, texnum);
    if (!tex) return;
    RrTextureFreeContents(tex);
    tex->type = RR_TEXTURE_RGBA;
    tex->data.rgba.x = x;
    tex->data.rgba.y = y;
    tex->data.rgba.w = w;
    tex->data.rgba.h = h;
    glGenTextures(1, &num);
    tex->data.rgba.texid = num;
    glBindTexture(GL_TEXTURE_2D, num);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void RrTextureSetText(struct RrSurface *sur,
                      int texnum,
                      struct RrFont *font,
                      enum RrLayout layout,
                      struct RrColor *color,
                      const char *text)
{
    struct RrTexture *tex = RrSurfaceTexture(sur, texnum);
    int l;

    if (!tex) return;
    RrTextureFreeContents(tex);
    tex->type = RR_TEXTURE_TEXT;
    tex->data.text.font = font;
    tex->data.text.layout = layout;
    tex->data.text.color = *color;

    l = strlen(text) + 1;
    tex->data.text.string = malloc(l);
    memcpy(tex->data.text.string, text, l);
}

void RrTextureSetNone(struct RrSurface *sur,
                      int texnum)
{
    struct RrTexture *tex = RrSurfaceTexture(sur, texnum);

    if (!tex) return;
    RrTextureFreeContents(tex);
}

void RrTexturePaint(struct RrSurface *sur, struct RrTexture *tex,
                    int x, int y, int w, int h)
{
    glEnable(GL_TEXTURE_2D);
    
    switch (tex->type) {
    case RR_TEXTURE_NONE:
        break;
    case RR_TEXTURE_TEXT:
        RrFontRenderString(sur, tex->data.text.font, &tex->data.text.color,
                           tex->data.text.layout, tex->data.text.string,
                           x, y, w, h);
        break;
    case RR_TEXTURE_RGBA:
        glColor3f(1.0, 1.0, 1.0);
        glBindTexture(GL_TEXTURE_2D, tex->data.rgba.texid);
        glBegin(GL_TRIANGLES);
        glVertex2i(x, y);
        glVertex2i(x+w, y); 
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glVertex2i(x, y+h);
        glVertex2i(x, y);
        glEnd();

        break;
    }
    glDisable(GL_TEXTURE_2D);
}
