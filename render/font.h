#ifndef __font_h
#define __font_h
#include <X11/Xft/Xft.h>
#include "render.h"
#include "kernel/geom.h"

void font_startup(void);
ObFont *font_open(char *fontstring);
void font_close(ObFont *f);
int font_measure_string(ObFont *f, char *str, int shadow, int offset);
int font_height(ObFont *f, int shadow, int offset);
int font_max_char_width(ObFont *f);
void font_draw(XftDraw *d, TextureText *t, Rect *position);
#endif /* __font_h */
