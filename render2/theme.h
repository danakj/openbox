#ifndef __render_theme_h
#define __render_theme_h

#include "render.h"

struct RrTheme;

struct RrTheme *RrThemeLoad(struct RrInstance *inst, const char *name);
void RrThemeDestroy(struct RrTheme *theme);

struct RrTheme {
    struct RrInstance *inst;

    int bevel;
    int bwidth;
    int cbwidth;
    int handle_height;

    char *title_layout;

    struct RrFont *title_font;
    enum RrLayout title_justify;

    struct RrColor b_color;
    struct RrColor cb_color;
    struct RrColor cb_color_f; /* focused */
    struct RrColor title_color;
    struct RrColor title_color_f; /* focused */
    struct RrColor button_color;
    struct RrColor button_color_f; /* focused */
    struct RrColor menu_title_color;
    struct RrColor menu_item_color;
    struct RrColor menu_disabled_color;
    struct RrColor menu_hilite_color;

    struct RrSurface *max;
    struct RrSurface *max_f;     /* focused */
    struct RrSurface *max_p;     /* pressed */
    struct RrSurface *max_p_f;   /* pressed-focused */

    struct RrSurface *iconify;
    struct RrSurface *iconify_f;   /* focused */
    struct RrSurface *iconify_p;   /* pressed */
    struct RrSurface *iconify_p_f; /* pressed-focused */

    struct RrSurface *close;
    struct RrSurface *close_f;     /* focused */
    struct RrSurface *close_p;     /* pressed */
    struct RrSurface *close_p_f;   /* pressed-focused */

    struct RrSurface *desk;
    struct RrSurface *desk_f;     /* focused */
    struct RrSurface *desk_p;     /* pressed */
    struct RrSurface *desk_p_f;   /* pressed-focused */

    struct RrSurface *shade;
    struct RrSurface *shade_f;     /* focused */
    struct RrSurface *shade_p;     /* pressed */
    struct RrSurface *shade_p_f;   /* pressed-focused */

    struct RrSurface *icon;

    struct RrSurface *frame;

    struct RrSurface *plate;
    struct RrSurface *plate_f; /* focused */

    struct RrSurface *title;
    struct RrSurface *title_f; /* focused */

    struct RrSurface *label;
    struct RrSurface *label_f; /* focused */

    struct RrSurface *grip;
    struct RrSurface *grip_f; /* focused */

    struct RrSurface *handle;
    struct RrSurface *handle_f; /* focused */

    struct RrSurface *menu_title;
    struct RrSurface *menu_item;
    struct RrSurface *menu_disabled;
    struct RrSurface *menu_hilite;

    struct RrColor app_label_color;
    struct RrColor app_label_color_h; /* hilited */

    struct RrSurface *app_bg;
    struct RrSurface *app_bg_h; /* hilited */
    struct RrSurface *app_label;
    struct RrSurface *app_label_h; /* hilited */
    struct RrSurface *app_icon;
};

int RrThemeLabelHeight(struct RrTheme *t);
#define RrThemeTitleHeight(t) (RrThemeLabelHeight(t) + \
                               ((t)->bevel + (t)->bwidth) * 2)
#define RrThemeButtonSize(t)  (RrThemeLabelHeight(t) - (t)->bevel * 2)
#define RrThemeGripWidth(t)   (RrThemeButtonSize(t) * 2)

#endif
