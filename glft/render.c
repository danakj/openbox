#include "render.h"
#include "font.h"
#include "debug.h"
#include <glib.h>
#include <GL/glx.h>

#define TPOINTS 15.0

#define TOFLOAT(x) (((x) >> 6) + ((x) & 63)/64.0)

#include FT_OUTLINE_H

struct GlftWalkState {
    int drawing;
    float x, y;
};

static struct GlftWalkState state;

int GlftMoveToFunc(FT_Vector *to, void *user)
{
    state.x = TOFLOAT(to->x);
    state.y = TOFLOAT(to->y);
    if (state.drawing) {
        glEnd();
    }
    glBegin(GL_LINE_STRIP);
    glVertex2f(state.x, state.y);
    state.drawing = 1;
    return 0;
}

int GlftLineToFunc(FT_Vector *to, void *user)
{
    state.x = TOFLOAT(to->x);
    state.y = TOFLOAT(to->y);
    glVertex2f(state.x, state.y);
    return 0;
}

int GlftConicToFunc(FT_Vector *c, FT_Vector *to, void *user)
{
    float t, u, x, y;

    for (t = 0, u = 1; t < 1.0; t += 1.0/TPOINTS, u = 1.0-t) {
        x = u*u*state.x + 2*t*u*TOFLOAT(c->x) + t*t*TOFLOAT(to->x);
        y = u*u*state.y + 2*t*u*TOFLOAT(c->y) + t*t*TOFLOAT(to->y);
        glVertex2f(x, y);
    }
    state.x = TOFLOAT(to->x);
    state.y = TOFLOAT(to->y);
    glVertex2f(state.x, state.y);
    return 0;
}

int GlftCubicToFunc(FT_Vector *c1, FT_Vector *c2, FT_Vector *to, void 
*user)
{
    GlftLineToFunc(to, user);
    g_message("cubic not currently rendered properly\n");
    return 0;
}

FT_Outline_Funcs GlftFuncs = {
    GlftMoveToFunc,
    GlftLineToFunc,
    GlftConicToFunc,
    GlftCubicToFunc,
    0,
    0
};

void GlftRenderGlyph(FT_Face face, unsigned int dlist)
{
    int err;
    FT_GlyphSlot slot = face->glyph;

    state.x = 0;
    state.y = 0;
    state.drawing = 0;

    glNewList(dlist, GL_COMPILE);
    err = FT_Outline_Decompose(&slot->outline, &GlftFuncs, NULL);
    g_assert(!err);
    if (state.drawing)
        glEnd();
    glEndList();
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
            glCallList(g->dlist);
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
