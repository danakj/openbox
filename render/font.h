#ifndef __font_h
#define __font_h
#define _XFT_NO_COMPAT_ /* no Xft 1 API */
#include <X11/Xft/Xft.h>
#include "render.h"
#include "kernel/geom.h"

struct _RrFont {
    const RrInstance *inst;
    XftFont *xftfont;
    gint elipses_length;
    gint shadow;
    gchar tint;
    gint offset;
};

RrFont *RrFontOpen(const RrInstance *inst, char *fontstring);
void RrFontClose(RrFont *f);
void RrFontDraw(XftDraw *d, RrTextureText *t, Rect *position);
#endif /* __font_h */
