#ifndef __render_instance_h
#define __render_instance_h

#include <X11/Xlib.h>
#include <glib.h>

struct _RrInstance {
    Display *display;
    gint screen;

    Visual *visual;
    gint depth;
    Colormap colormap;

    gint red_offset;
    gint green_offset;
    gint blue_offset;

    gint red_shift;
    gint green_shift;
    gint blue_shift;

    gint red_mask;
    gint green_mask;
    gint blue_mask;

    gint pseudo_bpc;
    XColor *pseudo_colors;

    GHashTable *color_hash;
};

guint       RrPseudoBPC    (const RrInstance *inst);
XColor*     RrPseudoColors (const RrInstance *inst);
GHashTable* RrColorHash    (const RrInstance *inst);

#endif
