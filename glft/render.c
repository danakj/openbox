#include "render.h"
#include "font.h"
#include "debug.h"
#include <glib.h>
#include <GL/glx.h>

#define TPOINTS 15.0

#define TOFLOAT(x) (((x) >> 6) + ((x) & 63)/64.0)

void GlftRenderGlyph(FT_Face face, struct GlftGlyph *g)
{
    unsigned char *padbuf;
    int err, i;
    FT_GlyphSlot slot = face->glyph;

    err = FT_Render_Glyph(slot, ft_render_mode_normal);
        g_assert(!err);

    g->texw = slot->bitmap.width;
    g->texh = slot->bitmap.rows;

    g->padx = 1;
    while (g->padx < slot->bitmap.width)
       g->padx <<= 1;

    g->pady = 1;
    while (g->pady < slot->bitmap.rows)
       g->pady <<= 1;

    padbuf = g_new0(unsigned char, g->padx * g->pady);
printf("glyph rendered to %d wide\n", slot->bitmap.width);
    for (i = 0; i < slot->bitmap.rows; i++)
        memcpy(padbuf + i*g->padx,
               slot->bitmap.buffer + i*slot->bitmap.width,
               slot->bitmap.width);
    glBindTexture(GL_TEXTURE_2D, g->tnum);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g->padx, g->pady,
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
printf("glyph width %d(%d)\n", g->width, g->x);
            glTranslatef(GlftFontAdvance(font, p, g), 0.0, 0.0);
            glBindTexture(GL_TEXTURE_2D, g->tnum);
            glBegin(GL_QUADS);
            glColor3f(0.0, 0.0, 0.0);

            glTexCoord2f(0, g->texh/(float)g->pady);
            glVertex2i(0, 0);

            glTexCoord2f(g->texw/(float)g->padx, g->texh/(float)g->pady);
            glVertex2i(0 + g->texw, 0);

            glTexCoord2f(g->texw/(float)g->padx, 0);
            glVertex2i(0 + g->texw, 0 + g->texh);

            glTexCoord2f(0, 0);
            glVertex2i(0, 0 + g->texh);
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
