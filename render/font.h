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
};

void font_startup(void);
RrFont *font_open(const RrInstance *inst, char *fontstring);
void font_close(RrFont *f);
int font_measure_string(RrFont *f, char *str, int shadow, int offset);
int font_height(RrFont *f, int shadow, int offset);
int font_max_char_width(RrFont *f);
void font_draw(XftDraw *d, RrTextureText *t, Rect *position);
#endif /* __font_h */
