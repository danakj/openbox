#include "render.h"
#include "font.h"
#include "debug.h"
#include <glib.h>
#include <GL/glx.h>

void GlftRenderGlyph(FT_Face face, unsigned int dlist)
{
    FT_GlyphSlot slot = face->glyph;
}

void GlftRenderString(struct GlftFont *font, const char *str, int bytes,
                      int x, int y)
{
    const char *c;
    struct GlftGlyph *g;

    if (!g_utf8_validate(str, bytes, NULL)) {
        GlftDebug("Invalid UTF-8 in string\n");
        return;
    }

    glPushMatrix();

    c = str;
    while (c) {
        g = GlftFontGlyph(font, c);
        if (g) {
            glCallList(g->dlist);
            glTranslatef(g->width, 0.0, 0.0);
        } else
            glTranslatef(font->max_advance_width, 0.0, 0.0);
        c = g_utf8_next_char(c);
        if (c - str >= bytes) break;
    }

    glPopMatrix();
}
