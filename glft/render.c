#include "render.h"
#include "font.h"
#include "debug.h"
#include <glib.h>
#include <GL/glx.h>

#define TPOINTS 15.0

#define TOFLOAT(x) (((x) >> 6) + ((x) & 63)/64.0)

void GlftRenderGlyph(FT_Face face, unsigned int tnum)
{
    unsigned char *padbuf;
    int padx = 1, pady = 1, i;
    int err;
    FT_GlyphSlot slot = face->glyph;

    err = FT_Render_Glyph( slot, ft_render_mode_normal );
        g_assert(!err);
    printf("bitmap with dims %d, %d\n", slot->bitmap.rows, 
           slot->bitmap.width);
    while (padx < slot->bitmap.width)
       padx <<= 1;
    while (pady < slot->bitmap.rows)
       pady <<= 1;
    printf("padding to %d, %d\n", padx, pady);
    padbuf = g_new(unsigned char, padx * pady);
    for (i = 0; i < slot->bitmap.rows; i++)
        memcpy(padbuf + i*padx,
               slot->bitmap.buffer + i*slot->bitmap.width,
               slot->bitmap.width);
    glBindTexture(GL_TEXTURE_2D, tnum);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, padx, pady,
                 0, GL_GREEN, GL_UNSIGNED_BYTE, padbuf);

    g_free(padbuf);
}

void GlftRenderString(struct GlftFont *font, const char *str, int bytes,
                      int x, int y)
{
    const char *c;
    struct GlftGlyph *g, *p = NULL;

    if (!g_utf8_validate(str, bytes, NULL)) {
        GlftDebug("Invalid UTF-8 in string\n");
        return;
    }

    glPushMatrix();

    c = str;
    while (c - str < bytes) {
        g = GlftFontGlyph(font, c);
        if (g) {
            glTranslatef(GlftFontAdvance(font, p, g), 0.0, 0.0);
            glBindTexture(GL_TEXTURE_2D, g->tnum);
            glBegin(GL_QUADS);
            glColor3f(1.0, 1.0, 1.0);

            glTexCoord2i(0, 1);
            glVertex2i(g->x, g->y);

            glTexCoord2i(1, 1);
            glVertex2i(g->x + g->width, g->y);

            glTexCoord2i(1, 0);
            glVertex2i(g->x + g->width ,g->y + g->height);

            glTexCoord2i(0, 0);
            glVertex2i(g->x, g->y + g->height);
            glEnd();
        } else
            glTranslatef(font->max_advance_width, 0.0, 0.0);
        p = g;
        c = g_utf8_next_char(c);
    }

    glPopMatrix();
}

void GlftMeasureString(struct GlftFont *font,
                       const char *str,
                       int bytes,
                       int *w,
                       int *h)
{
    const char *c;
    struct GlftGlyph *g, *p = NULL;

    if (!g_utf8_validate(str, bytes, NULL)) {
        GlftDebug("Invalid UTF-8 in string\n");
        return;
    }

    *w = 0;
    *h = 0;

    c = str;
    while (c - str < bytes) {
        g = GlftFontGlyph(font, c);
        if (g) {
            *w += GlftFontAdvance(font, p, g);
            *h = MAX(g->height, *h);
        } else
            *w += font->max_advance_width;
        p = g;
        c = g_utf8_next_char(c);
    }
}
