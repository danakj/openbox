#include "render.h"
#include "theme.h"
#include "planar.h"
#include <stdlib.h>

struct RrTheme *RrThemeLoad(struct RrInstance *inst, const char *name)
{
    struct RrTheme *theme;
    struct RrColor pri, sec, bor;

    theme = malloc(sizeof(struct RrTheme));
    theme->inst = inst;

#define MERRY
#ifdef MERRY

    theme->bevel = 1;
    theme->bwidth = 1;
    theme->cbwidth = 0;
    theme->handle_height = 4;

    theme->title_layout = "NLIMC";

    theme->title_font = RrFontOpen(inst,
                                  "arial:bold:pixelsize=11:"
                                  "shadow=true:shadowoffset=1:shadowtint=0.1");
    theme->title_justify = RR_CENTER;

    RrColorSet(&theme->b_color, 0, 0, 0, 1);
    RrColorSet(&theme->cb_color, 0, 0, 0, 1);
    RrColorSet(&theme->cb_color_f, 0, 0, 0, 1);
    RrColorSet(&theme->title_color, 1, 1, 1, 1);
    RrColorSet(&theme->title_color_f, 1, 1, 1, 1);
    RrColorSet(&theme->button_color, 0.3, 0.3, 0.3, 1);
    RrColorSet(&theme->button_color_f, 0.12, 0.12, 0.12, 1);
    RrColorSet(&theme->menu_title_color, 1, 1, 1, 1);
    RrColorSet(&theme->menu_item_color, 1, 1, 1, 1);
    RrColorSet(&theme->menu_disabled_color, 1, 1, 1, 1);
    RrColorSet(&theme->menu_hilite_color, 1, 1, 1, 1);

    theme->max = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->max_f = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->max_p = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->max_p_f = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->iconify = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->iconify_f = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->iconify_p = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->iconify_p_f = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->close = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->close_f = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->close_p = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->close_p_f = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->desk = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->desk_f = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->desk_p = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->desk_p_f = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->shade = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->shade_f = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->shade_p = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->shade_p_f = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);

    RrColorSet(&pri, 0.9, 0.91, 0.9, 1);
    RrPlanarSet(theme->max, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);
    RrPlanarSet(theme->max_f, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);

    RrColorSet(&pri, 0.71, 0.7, 0.68, 1);
    RrPlanarSet(theme->max_p, RR_PLANAR_SOLID, RR_SUNKEN_OUTER,
                &pri, NULL, 0, NULL);
    RrPlanarSet(theme->max_p_f, RR_PLANAR_SOLID, RR_SUNKEN_OUTER,
                &pri, NULL, 0, NULL);

    RrColorSet(&pri, 0.9, 0.91, 0.9, 1);
    RrPlanarSet(theme->iconify, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);
    RrPlanarSet(theme->iconify_f, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);

    RrColorSet(&pri, 0.71, 0.7, 0.68, 1);
    RrPlanarSet(theme->iconify_p, RR_PLANAR_SOLID, RR_SUNKEN_OUTER,
                &pri, NULL, 0, NULL);
    RrPlanarSet(theme->iconify_p_f, RR_PLANAR_SOLID, RR_SUNKEN_OUTER,
                &pri, NULL, 0, NULL);

    RrColorSet(&pri, 0.9, 0.91, 0.9, 1);
    RrPlanarSet(theme->close, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);
    RrPlanarSet(theme->close_f, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);

    RrColorSet(&pri, 0.71, 0.7, 0.68, 1);
    RrPlanarSet(theme->close_p, RR_PLANAR_SOLID, RR_SUNKEN_OUTER,
                &pri, NULL, 0, NULL);
    RrPlanarSet(theme->close_p_f, RR_PLANAR_SOLID, RR_SUNKEN_OUTER,
                &pri, NULL, 0, NULL);

    RrColorSet(&pri, 0.9, 0.91, 0.9, 1);
    RrPlanarSet(theme->desk, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);
    RrPlanarSet(theme->desk_f, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);

    RrColorSet(&pri, 0.71, 0.7, 0.68, 1);
    RrPlanarSet(theme->desk_p, RR_PLANAR_SOLID, RR_SUNKEN_OUTER,
                &pri, NULL, 0, NULL);
    RrPlanarSet(theme->desk_p_f, RR_PLANAR_SOLID, RR_SUNKEN_OUTER,
                &pri, NULL, 0, NULL);

    RrColorSet(&pri, 0.9, 0.91, 0.9, 1);
    RrPlanarSet(theme->shade, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);
    RrPlanarSet(theme->shade_f, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);

    RrColorSet(&pri, 0.71, 0.7, 0.68, 1);
    RrPlanarSet(theme->shade_p, RR_PLANAR_SOLID, RR_SUNKEN_OUTER,
                &pri, NULL, 0, NULL);
    RrPlanarSet(theme->shade_p_f, RR_PLANAR_SOLID, RR_SUNKEN_OUTER,
                &pri, NULL, 0, NULL);

    theme->frame = RrSurfaceNewProto(RR_SURFACE_PLANAR, 0);

    RrColorSet(&pri, 0.9, 0.91, 0.9, 1);
    RrColorSet(&bor, 0, 0, 0, 1);
    RrPlanarSet(theme->frame, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 1, &bor);

    theme->title = RrSurfaceNewProto(RR_SURFACE_PLANAR, 0);
    theme->title_f = RrSurfaceNewProto(RR_SURFACE_PLANAR, 0);

    RrColorSet(&pri, 0.9, 0.91, 0.9, 1);
    RrColorSet(&bor, 0, 0, 0, 1);
    RrPlanarSet(theme->title, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 1, &bor);
    RrPlanarSet(theme->title_f, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 1, &bor);

    theme->plate = RrSurfaceNewProto(RR_SURFACE_PLANAR, 0);
    theme->plate_f = RrSurfaceNewProto(RR_SURFACE_PLANAR, 0);

    RrColorSet(&pri, 0, 0, 0, 1);
    RrPlanarSet(theme->plate, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);
    RrPlanarSet(theme->plate_f, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);

    theme->label = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->label_f = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);

    RrColorSet(&pri, 0.70, 0.70, 0.68, 1);
    RrColorSet(&sec, 0.75, 0.73, 0.71, 1);
    RrColorSet(&bor, 0.42, 0.41, 0.42, 1);
    RrPlanarSet(theme->label, RR_PLANAR_VERTICAL, RR_BEVEL_NONE,
                &pri, &sec, 1, &bor);
    RrColorSet(&pri, 0.30, 0.34, 0.65, 1);
    RrColorSet(&sec, 0.35, 0.43, 0.75, 1);
    RrColorSet(&bor, 0, 0, 0, 1);
    RrPlanarSet(theme->label_f, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, &sec, 1, &bor);


    theme->grip = RrSurfaceNewProto(RR_SURFACE_PLANAR, 0);
    theme->grip_f = RrSurfaceNewProto(RR_SURFACE_PLANAR, 0);

    RrColorSet(&pri, 0.9, 0.91, 0.9, 1);
    RrColorSet(&bor, 0, 0, 0, 1);
    RrPlanarSet(theme->grip, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 1, &bor);
    RrPlanarSet(theme->grip_f, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 1, &bor);

    theme->handle = RrSurfaceNewProto(RR_SURFACE_PLANAR, 0);
    theme->handle_f = RrSurfaceNewProto(RR_SURFACE_PLANAR, 0);

    RrColorSet(&pri, 0.9, 0.91, 0.9, 1);
    RrColorSet(&bor, 0, 0, 0, 1);
    RrPlanarSet(theme->handle, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 1, &bor);
    RrPlanarSet(theme->handle_f, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 1, &bor);

    theme->menu_title = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);

    RrColorSet(&pri, 0.29, 0.35, 0.65, 1);
    RrColorSet(&bor, 0, 0, 0, 1);
    RrPlanarSet(theme->menu_title, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 1, &bor);

    theme->menu_item = RrSurfaceNewProto(RR_SURFACE_PLANAR, 0);

    RrColorSet(&pri, 0.9, 0.91, 0.9, 1);
    RrPlanarSet(theme->menu_item, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);

    theme->menu_disabled = RrSurfaceNewProto(RR_SURFACE_PLANAR, 0);

    RrColorSet(&pri, 0.9, 0.91, 0.9, 1);
    RrPlanarSet(theme->menu_item, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);

    theme->menu_hilite = RrSurfaceNewProto(RR_SURFACE_PLANAR, 0);

    /* app stuff */

    RrColorSet(&theme->app_label_color, 1, 1, 1, 1);
    RrColorSet(&theme->app_label_color_h, 1, 1, 1, 1);

    RrColorSet(&pri, 0.9, 0.91, 0.9, 1);
    RrPlanarSet(theme->menu_item, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);

    theme->app_bg = RrSurfaceNewProto(RR_SURFACE_PLANAR, 0);
    theme->app_bg_h = RrSurfaceNewProto(RR_SURFACE_PLANAR, 0);

    RrColorSet(&pri, 0.9, 0.91, 0.9, 1);
    RrColorSet(&bor, 0, 0, 0, 1);
    RrPlanarSet(theme->app_bg, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 1, &bor);
    RrPlanarSet(theme->app_bg_h, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 1, &bor);

    theme->app_label = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    theme->app_label_h = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);

    RrColorSet(&pri, 0.70, 0.70, 0.68, 1);
    RrColorSet(&sec, 0.75, 0.73, 0.71, 1);
    RrColorSet(&bor, 0.42, 0.41, 0.42, 1);
    RrPlanarSet(theme->app_label, RR_PLANAR_VERTICAL, RR_BEVEL_NONE,
                &pri, &sec, 1, &bor); 
    RrColorSet(&pri, 0.30, 0.34, 0.65, 1);
    RrColorSet(&sec, 0.35, 0.43, 0.75, 1);
    RrColorSet(&bor, 0, 0, 0, 1);
    RrPlanarSet(theme->app_label_h, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, &sec, 1, &bor);

#endif

    RrColorSet(&pri, 0, 0, 0, 0); /* clear */

    theme->icon = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    RrPlanarSet(theme->icon, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);

    theme->app_icon = RrSurfaceNewProto(RR_SURFACE_PLANAR, 1);
    RrPlanarSet(theme->app_icon, RR_PLANAR_SOLID, RR_BEVEL_NONE,
                &pri, NULL, 0, NULL);

    return theme;
}

void RrThemeDestroy(struct RrTheme *theme)
{
    if (theme) {
        RrFontClose(theme->title_font);

        RrSurfaceFree(theme->max);
        RrSurfaceFree(theme->max_f);
        RrSurfaceFree(theme->max_p);
        RrSurfaceFree(theme->max_p_f);

        RrSurfaceFree(theme->iconify);
        RrSurfaceFree(theme->iconify_f);
        RrSurfaceFree(theme->iconify_p);
        RrSurfaceFree(theme->iconify_p_f);

        RrSurfaceFree(theme->close);
        RrSurfaceFree(theme->close_f);
        RrSurfaceFree(theme->close_p);
        RrSurfaceFree(theme->close_p_f);

        RrSurfaceFree(theme->desk);
        RrSurfaceFree(theme->desk_f);
        RrSurfaceFree(theme->desk_p);
        RrSurfaceFree(theme->desk_p_f);

        RrSurfaceFree(theme->shade);
        RrSurfaceFree(theme->shade_f);
        RrSurfaceFree(theme->shade_p);
        RrSurfaceFree(theme->shade_p_f);

        RrSurfaceFree(theme->icon);

        RrSurfaceFree(theme->frame);

        RrSurfaceFree(theme->title);
        RrSurfaceFree(theme->title_f);

        RrSurfaceFree(theme->label);
        RrSurfaceFree(theme->label_f);

        RrSurfaceFree(theme->grip);
        RrSurfaceFree(theme->grip_f);

        RrSurfaceFree(theme->handle);
        RrSurfaceFree(theme->handle_f);

        RrSurfaceFree(theme->menu_title);
        RrSurfaceFree(theme->menu_item);
        RrSurfaceFree(theme->menu_disabled);
        RrSurfaceFree(theme->menu_hilite);

        RrSurfaceFree(theme->app_bg);
        RrSurfaceFree(theme->app_bg_h);
        RrSurfaceFree(theme->app_label);
        RrSurfaceFree(theme->app_label_h);
        RrSurfaceFree(theme->app_icon);

        free(theme);
    }
}

int RrThemeLabelHeight(struct RrTheme *t)
{
    int h;
    h = RrFontHeight(t->title_font);
    h += 2 * MAX(RrPlanarEdgeWidth(t->label),
                 RrPlanarEdgeWidth(t->label_f));
    return h;
}
