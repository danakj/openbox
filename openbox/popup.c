#include "popup.h"

#include "openbox.h"
#include "frame.h"
#include "client.h"
#include "window.h"
#include "stacking.h"
#include "render/render.h"
#include "render/theme.h"

struct _ObPopup
{
    ObWindow obwin;
    Window bg;

    Window icon;
    Window text;

    gboolean hasicon;
    RrAppearance *a_bg;
    RrAppearance *a_icon;
    RrAppearance *a_text;
    gint gravity;
    gint x;
    gint y;
    gint w;
    gint h;
    gboolean mapped;
};

Popup *popup_new(gboolean hasicon)
{
    XSetWindowAttributes attrib;
    Popup *self = g_new(Popup, 1);

    self->obwin.type = Window_Internal;
    self->hasicon = hasicon;
    self->a_text = NULL;
    self->gravity = NorthWestGravity;
    self->x = self->y = self->w = self->h = 0;
    self->mapped = FALSE;
    self->a_bg = self->a_icon = self->a_text = NULL;

    attrib.override_redirect = True;
    self->bg = XCreateWindow(ob_display, RootWindow(ob_display, ob_screen),
                             0, 0, 1, 1, 0, RrDepth(ob_rr_inst),
                             InputOutput, RrVisual(ob_rr_inst),
                             CWOverrideRedirect, &attrib);
    
    self->text = XCreateWindow(ob_display, self->bg,
                               0, 0, 1, 1, 0, RrDepth(ob_rr_inst),
                               InputOutput, RrVisual(ob_rr_inst), 0, NULL);

    if (self->hasicon)
        self->icon = XCreateWindow(ob_display, self->bg,
                                   0, 0, 1, 1, 0,
                                   RrDepth(ob_rr_inst), InputOutput,
                                   RrVisual(ob_rr_inst), 0, NULL);

    XMapWindow(ob_display, self->text);
    XMapWindow(ob_display, self->icon);

    stacking_add(INTERNAL_AS_WINDOW(self));
    return self;
}

void popup_free(Popup *self)
{
    if (self) {
        XDestroyWindow(ob_display, self->bg);
        XDestroyWindow(ob_display, self->text);
        XDestroyWindow(ob_display, self->icon);
        RrAppearanceFree(self->a_bg);
        RrAppearanceFree(self->a_icon);
        RrAppearanceFree(self->a_text);
        stacking_remove(self);
        g_free(self);
    }
}

void popup_position(Popup *self, gint gravity, gint x, gint y)
{
    self->gravity = gravity;
    self->x = x;
    self->y = y;
}

void popup_size(Popup *self, gint w, gint h)
{
    self->w = w;
    self->h = h;
}

void popup_size_to_string(Popup *self, gchar *text)
{
    gint textw, texth;
    gint iconw;

    if (!self->a_text)
        self->a_text = RrAppearanceCopy(ob_rr_theme->app_selected_label);

    self->a_text->texture[0].data.text.string = text;
    RrMinsize(self->a_text, &textw, &texth);
    textw += ob_rr_theme->bevel * 2;
    texth += ob_rr_theme->bevel * 2;

    self->h = texth + ob_rr_theme->bevel * 2;
    iconw = (self->hasicon ? texth : 0);
    self->w = textw + iconw + ob_rr_theme->bevel * (self->hasicon ? 3 : 2);
}

void popup_set_text_align(Popup *self, RrJustify align)
{
    if (!self->a_text)
        self->a_text = RrAppearanceCopy(ob_rr_theme->app_selected_label);

    self->a_text->texture[0].data.text.justify = align;
}

void popup_show(Popup *self, gchar *text, ObClientIcon *icon)
{
    gint x, y, w, h;
    gint textw, texth;
    gint iconw;

    /* create the shit if needed */
    if (!self->a_bg)
        self->a_bg = RrAppearanceCopy(ob_rr_theme->app_selected_bg);
    if (self->hasicon && !self->a_icon)
        self->a_icon = RrAppearanceCopy(ob_rr_theme->a_clear_tex);
    if (!self->a_text)
        self->a_text = RrAppearanceCopy(ob_rr_theme->app_selected_label);

    XSetWindowBorderWidth(ob_display, self->bg, ob_rr_theme->bwidth);
    XSetWindowBorder(ob_display, self->bg, ob_rr_theme->b_color->pixel);

    /* set up the textures */
    self->a_text->texture[0].data.text.string = text;
    if (self->hasicon) {
        if (icon) {
            self->a_icon->texture[0].type = RR_TEXTURE_RGBA;
            self->a_icon->texture[0].data.rgba.width = icon->width;
            self->a_icon->texture[0].data.rgba.height = icon->height;
            self->a_icon->texture[0].data.rgba.data = icon->data;
        } else
            self->a_icon->texture[0].type = RR_TEXTURE_NONE;
    }

    /* measure the shit out */
    RrMinsize(self->a_text, &textw, &texth);
    textw += ob_rr_theme->bevel * 2;
    texth += ob_rr_theme->bevel * 2;

    /* set the sizes up and reget the text sizes from the calculated
       outer sizes */
    if (self->h) {
        h = self->h;
        texth = h - (ob_rr_theme->bevel * 2);
    } else
        h = texth + ob_rr_theme->bevel * 2;
    iconw = (self->hasicon ? texth : 0);
    if (self->w) {
        w = self->w;
        textw = w - (iconw + ob_rr_theme->bevel * (self->hasicon ? 3 : 2));
    } else
        w = textw + iconw + ob_rr_theme->bevel * (self->hasicon ? 3 : 2);
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

    /* set the windows/appearances up */
    XMoveResizeWindow(ob_display, self->bg, x, y, w, h);

    self->a_text->surface.parent = self->a_bg;
    self->a_text->surface.parentx = iconw +
        ob_rr_theme->bevel * (self->hasicon ? 2 : 1);
    self->a_text->surface.parenty = ob_rr_theme->bevel;
    XMoveResizeWindow(ob_display, self->text,
                      iconw + ob_rr_theme->bevel * (self->hasicon ? 2 : 1),
                      ob_rr_theme->bevel, textw, texth);

    if (self->hasicon) {
        if (iconw < 1) iconw = 1; /* sanity check for crashes */
        self->a_icon->surface.parent = self->a_bg;
        self->a_icon->surface.parentx = ob_rr_theme->bevel;
        self->a_icon->surface.parenty = ob_rr_theme->bevel;
        XMoveResizeWindow(ob_display, self->icon,
                          ob_rr_theme->bevel, ob_rr_theme->bevel,
                          iconw, texth);
    }

    RrPaint(self->a_bg, self->bg, w, h);
    RrPaint(self->a_text, self->text, textw, texth);
    if (self->hasicon)
        RrPaint(self->a_icon, self->icon, iconw, texth);

    if (!self->mapped) {
        XMapWindow(ob_display, self->bg);
        stacking_raise(INTERNAL_AS_WINDOW(self));
        self->mapped = TRUE;
    }
}

void popup_hide(Popup *self)
{
    if (self->mapped) {
        XUnmapWindow(ob_display, self->bg);
        self->mapped = FALSE;
    }
}
