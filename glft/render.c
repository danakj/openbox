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
    printf("move to %f:%f\n", state.x, state.y);
    if (state.drawing) {
        glEnd();
        state.drawing = 0;
    }
    return 0;
}

int GlftLineToFunc(FT_Vector *to, void *user)
{
    if (!state.drawing) {
        glBegin(GL_LINES);
        glVertex2f(state.x, state.y);
        state.drawing = 1;
    } else
        glVertex2f(state.x, state.y);
    state.x = TOFLOAT(to->x);
    state.y = TOFLOAT(to->y);
    printf("line to %f:%f\n", state.x, state.y);
    glVertex2f(state.x, state.y);
    return 0;
}

int GlftConicToFunc(FT_Vector *c, FT_Vector *to, void *user)
{
    float t, u, x, y;

    for (t = 0, u = 1; t < 1.0; t += 1.0/TPOINTS, u = 1.0-t) {
        x = u*u*state.x + 2*t*u*TOFLOAT(c->x) + t*t*TOFLOAT(to->x);
        y = u*u*state.y + 2*t*u*TOFLOAT(c->y) + t*t*TOFLOAT(to->y);
/*printf("cone to %f, %f (%f, %f)\n", x, y, t, u);*/
        glVertex2f(x, y);
    }
    state.x = TOFLOAT(to->x);
    state.y = TOFLOAT(to->y);
    printf("conic the hedgehog!\n");
    return 0;
}

int GlftCubicToFunc(FT_Vector *c1, FT_Vector *c2, FT_Vector *to, void 
*user)
{
    GlftLineToFunc(to, user);
    printf("cubic\n");
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
    struct GlftGlyph *g;

    if (!g_utf8_validate(str, bytes, NULL)) {
        GlftDebug("Invalid UTF-8 in string\n");
        return;
    }

    glPushMatrix();

    c = str;
    while (c - str < bytes) {
        g = GlftFontGlyph(font, c);
        if (g) {
            glCallList(g->dlist);
            glTranslatef(g->width, 0.0, 0.0);
        } else
            glTranslatef(font->max_advance_width, 0.0, 0.0);
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
    struct GlftGlyph *g;

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
            *w += g->width;
            *h = MAX(g->height, *h);
        } else {
            *w += font->max_advance_width;
        }
        c = g_utf8_next_char(c);
    }
}
