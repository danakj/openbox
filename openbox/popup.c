#include "openbox.h"
#include "frame.h"
#include "window.h"
#include "stacking.h"
#include "render/render.h"
#include "render/theme.h"

typedef struct Popup {
    ObWindow obwin;
    Window bg;

    Window icon;
    Window text;

    gboolean hasicon;
    Appearance *a_bg;
    Appearance *a_icon;
    Appearance *a_text;
    int gravity;
    int x;
    int y;
    int w;
    int h;
    gboolean mapped;
} Popup;

Popup *popup_new(gboolean hasicon)
{
    Popup *self = g_new(Popup, 1);
    self->obwin.type = Window_Internal;
    self->hasicon = hasicon;
    self->bg = None;
    self->a_text = NULL;
    self->gravity = NorthWestGravity;
    self->x = self->y = self->w = self->h = 0;
    self->mapped = FALSE;
    stacking_add(INTERNAL_AS_WINDOW(self));
    stacking_raise(INTERNAL_AS_WINDOW(self));
    return self;
}

void popup_free(Popup *self)
{
    if (self->bg) {
        XDestroyWindow(ob_display, self->bg);
        XDestroyWindow(ob_display, self->text);
        XDestroyWindow(ob_display, self->icon);
        appearance_free(self->a_bg);
        if (self->hasicon)
            appearance_free(self->a_icon);
    }
    if (self->a_text)
        appearance_free(self->a_text);
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

    if (!self->a_text)
        self->a_text = appearance_copy(theme_app_hilite_label);

    self->a_text->texture[0].data.text.string = text;
    appearance_minsize(self->a_text, &textw, &texth);
    textw += theme_bevel * 2;
    texth += theme_bevel * 2;

    self->h = texth + theme_bevel * 2;
    iconw = (self->hasicon ? texth : 0);
    self->w = textw + iconw + theme_bevel * (self->hasicon ? 3 : 2);
}

void popup_show(Popup *self, char *text, Icon *icon)
{
    XSetWindowAttributes attrib;
    int x, y, w, h;
    int textw, texth;
    int iconw;

    /* create the shit if needed */
    if (!self->bg) {
        attrib.override_redirect = True;
        self->bg = XCreateWindow(ob_display, ob_root,
                                 0, 0, 1, 1, 0, render_depth, InputOutput,
                                 render_visual, CWOverrideRedirect, &attrib);

        XSetWindowBorderWidth(ob_display, self->bg, theme_bwidth);
        XSetWindowBorder(ob_display, self->bg, theme_b_color->pixel);

        self->text = XCreateWindow(ob_display, self->bg,
                                   0, 0, 1, 1, 0, render_depth, InputOutput,
                                   render_visual, 0, NULL);
        if (self->hasicon)
            self->icon = XCreateWindow(ob_display, self->bg,
                                       0, 0, 1, 1, 0,
                                       render_depth, InputOutput,
                                       render_visual, 0, NULL);

        XMapWindow(ob_display, self->text);
        XMapWindow(ob_display, self->icon);

        self->a_bg = appearance_copy(theme_app_hilite_bg);
        if (self->hasicon)
            self->a_icon = appearance_copy(theme_app_icon);
    }
    if (!self->a_text)
        self->a_text = appearance_copy(theme_app_hilite_label);

    /* set up the textures */
    self->a_text->texture[0].data.text.string = text;
    if (self->hasicon) {
        if (icon) {
            self->a_icon->texture[0].type = RGBA;
            self->a_icon->texture[0].data.rgba.width = icon->width;
            self->a_icon->texture[0].data.rgba.height = icon->height;
            self->a_icon->texture[0].data.rgba.data = icon->data;
        } else
            self->a_icon->texture[0].type = NoTexture;
    }

    /* measure the shit out */
    appearance_minsize(self->a_text, &textw, &texth);
    textw += theme_bevel * 2;
    texth += theme_bevel * 2;

    /* set the sizes up and reget the text sizes from the calculated
       outer sizes */
    if (self->h) {
        h = self->h;
        texth = h - (theme_bevel * 2);
    } else
        h = texth + theme_bevel * 2;
    iconw = (self->hasicon ? texth : 0);
    if (self->w) {
        w = self->w;
        textw = w - (iconw + theme_bevel * (self->hasicon ? 3 : 2));
    } else
        w = textw + iconw + theme_bevel * (self->hasicon ? 3 : 2);
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
    RECT_SET(self->a_bg->area, 0, 0, w, h);
    XMoveResizeWindow(ob_display, self->bg, x, y, w, h);

    RECT_SET(self->a_text->area, 0, 0, textw, texth); 
    RECT_SET(self->a_text->texture[0].position, theme_bevel, theme_bevel,
             textw - theme_bevel * 2, texth - theme_bevel * 2);
    self->a_text->surface.data.planar.parent = self->a_bg;
    self->a_text->surface.data.planar.parentx = iconw +
        theme_bevel * (self->hasicon ? 2 : 1);
    self->a_text->surface.data.planar.parenty = theme_bevel;
    XMoveResizeWindow(ob_display, self->text,
                      iconw + theme_bevel * (self->hasicon ? 2 : 1),
                      theme_bevel, textw, texth);

    if (self->hasicon) {
        if (iconw < 1) iconw = 1; /* sanity check for crashes */
        RECT_SET(self->a_icon->area, 0, 0, iconw, texth);
        RECT_SET(self->a_icon->texture[0].position, 0, 0, iconw, texth);
        self->a_icon->surface.data.planar.parent = self->a_bg;
        self->a_icon->surface.data.planar.parentx = theme_bevel;
        self->a_icon->surface.data.planar.parenty = theme_bevel;
        XMoveResizeWindow(ob_display, self->icon,
                          theme_bevel, theme_bevel, iconw, texth);
    }

    paint(self->bg, self->a_bg);
    paint(self->text, self->a_text);
    if (self->hasicon)
        paint(self->icon, self->a_icon);

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
