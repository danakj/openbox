#include "render.h"
#include "font.h"
#include "debug.h"
#include <glib.h>
#include <GL/glx.h>

#include FT_OUTLINE_H

struct GlftWalkState {
    int drawing;
    float x, y;
};

static struct GlftWalkState state;

int GlftMoveToFunc(FT_Vector *to, void *user)
{
    state.x = (to->x >> 6) + (to->x & 63)/64;
    state.y = (to->y >> 6) + (to->y & 63)/64;
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
    state.x = (to->x >> 6) + (to->x & 63)/64;
    state.y = (to->y >> 6) + (to->y & 63)/64;
    printf("line to %f:%f\n", state.x, state.y);
    glVertex2f(state.x, state.y);
    return 0;
}

int GlftConicToFunc(FT_Vector *c, FT_Vector *to, void *user)
{
    GlftLineToFunc(to, user);
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
