#include "openbox.h"
#include "frame.h"
#include "window.h"
#include "stacking.h"
#include "render2/render.h"
#include "render2/theme.h"

typedef struct Popup {
    ObWindow obwin;
    Window bg;

    Window icon;
    Window text;

    gboolean hasicon;
    struct RrSurface *s_bg;
    struct RrSurface *s_icon;
    struct RrSurface *s_text;
    int gravity;
    int x;
    int y;
    int w;
    int h;
} Popup;

Popup *popup_new(gboolean hasicon)
{
    XSetWindowAttributes attrib;
    Popup *self;

    self = g_new(Popup, 1);
    self->obwin.type = Window_Internal;
    self->hasicon = hasicon;
    self->gravity = NorthWestGravity;
    self->x = self->y = self->w = self->h = 0;
    stacking_add(INTERNAL_AS_WINDOW(self));
    stacking_raise(INTERNAL_AS_WINDOW(self));

    attrib.override_redirect = True;
    attrib.event_mask = ExposureMask;
    self->bg = XCreateWindow(ob_display, ob_root,
                             0, 0, 1, 1, 0, RrInstanceDepth(ob_render_inst),
                             InputOutput, RrInstanceVisual(ob_render_inst),
                             CWEventMask|CWOverrideRedirect, &attrib);
    self->s_bg = RrSurfaceNew(ob_render_inst, 0, self->bg, 0);
    RrSurfaceCopy(self->s_bg, ob_theme->app_bg_h);

    self->s_text = RrSurfaceNewChild(0, self->s_bg, 1);
    RrSurfaceCopy(self->s_text, ob_theme->app_label_h);

    self->text = RrSurfaceWindow(self->s_text);

    if (self->hasicon) {
        self->s_icon = RrSurfaceNewChild(RR_SURFACE_PLANAR, self->s_bg, 1);
        RrSurfaceCopy(self->s_icon, ob_theme->app_icon);

        self->icon = RrSurfaceWindow(self->s_icon);
    } else {
        self->s_icon = NULL;
        self->icon = None;
    }

    return self;
}

void popup_free(Popup *self)
{
    RrSurfaceFree(self->s_icon);
    RrSurfaceFree(self->s_text);
    RrSurfaceFree(self->s_bg);
    XDestroyWindow(ob_display, self->bg);
    stacking_remove(self);
    g_free(self);
}

void popup_position(Popup *self, int gravity, int x, int y)
{
    self->gravity = gravity;
    self->x = x;
    self->y = y;
}

void popup_size(Popup *self, int w, int h)
{
    self->w = w;
    self->h = h;
}

void popup_size_to_string(Popup *self, char *text)
{
    int textw, texth;
    int iconw;

    RrTextureSetText(self->s_text, 0, ob_theme->title_font, RR_LEFT,
                     &ob_theme->app_label_color_h, text);
    RrSurfaceMinSize(self->s_text, &textw, &texth);
    textw += ob_theme->bevel * 2;
    texth += ob_theme->bevel * 2;

    self->h = texth + ob_theme->bevel * 2 + ob_theme->bwidth * 2;
    iconw = (self->hasicon ? texth + ob_theme->bevel : 0);
    self->w = textw + iconw + ob_theme->bevel * 2 + ob_theme->bwidth * 2;
}

void popup_show(Popup *self, char *text, Icon *icon)
{
    int x, y, w, h;
    int textw, texth;
    int iconw;

    /* set up the textures */
    RrTextureSetText(self->s_text, 0, ob_theme->title_font, RR_LEFT,
                     &ob_theme->app_label_color_h, text);

    /* measure the shit out */
    RrSurfaceMinSize(self->s_text, &textw, &texth);
    textw += ob_theme->bevel * 2;
    texth += ob_theme->bevel * 2;

    /* set the sizes up and reget the text sizes from the calculated
       outer sizes */
    if (self->h) {
        h = self->h;
        texth = h - (ob_theme->bevel * 2) - ob_theme->bwidth * 2;
    } else
        h = texth + ob_theme->bevel * 2 + ob_theme->bwidth * 2;
    iconw = (self->hasicon ? texth : 0);
    if (self->w) {
        w = self->w;
        textw = w - (iconw + ob_theme->bevel * (self->hasicon ? 3 : 2)) -
            ob_theme->bwidth * 2;
    } else
        w = textw + iconw + ob_theme->bevel * (self->hasicon ? 3 : 2) +
            ob_theme->bwidth * 2;
    /* sanity checks to avoid crashes! */
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    if (textw < 1) textw = 1;
    if (texth < 1) texth = 1;

    /* set up the x coord */
    x = self->x;
    switch (self->gravity) {
    case NorthGravity:
    case CenterGravity:
    case SouthGravity:
        x -= w / 2;
        break;
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
        x -= w;
        break;
    }

    /* set up the y coord */
    y = self->y;
    switch (self->gravity) {
    case WestGravity:
    case CenterGravity:
    case EastGravity:
        y -= h / 2;
        break;
    case SouthWestGravity:
    case SouthGravity:
    case SouthEastGravity:
        y -= h;
        break;
    }

    /* set the surfaces up */
    RrSurfaceSetArea(self->s_bg, x, y, w, h);

    RrSurfaceSetArea(self->s_text,
                     iconw + ob_theme->bevel * (self->hasicon ? 2 : 1) +
                     ob_theme->bwidth,
                     ob_theme->bevel + ob_theme->bwidth,
                     textw, texth);

    if (self->hasicon) {
        if (icon)
            RrTextureSetRGBA(self->s_icon, 0, icon->data, 0, 0, icon->width,
                             icon->height);
        else
            RrTextureSetNone(self->s_icon, 0);
        if (iconw < 1) iconw = 1; /* sanity check for crashes */
        RrSurfaceSetArea(self->s_icon,
                         ob_theme->bwidth + ob_theme->bevel,
                         ob_theme->bwidth + ob_theme->bevel,
                         iconw, iconw);
    }

    if (!RrSurfaceVisible(self->s_bg)) {
        RrSurfaceShow(self->s_bg);
        stacking_raise(INTERNAL_AS_WINDOW(self));
    } else {
        RrPaint(self->s_text, 1);
        if (self->hasicon)
            RrPaint(self->s_icon, 1);
    }
}

void popup_hide(Popup *self)
{
    RrSurfaceHide(self->s_bg);
}
