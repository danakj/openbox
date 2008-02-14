/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

 frame_default_config.c for the Openbox window manager
 Copyright (c) 2006        Mikael Magnusson
 Copyright (c) 2003-2007   Dana Jansens

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 See the COPYING file for a copy of the GNU General Public License.
 */

#include <X11/Xlib.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "render/render.h"
#include "render/color.h"
#include "render/font.h"
#include "render/mask.h"
#include "render/icon.h"
#include "obt/parse.h"

#include "config.h"
#include "plugin.h"

void load_pixmap(const gchar * theme_name, const gchar * base_name,
        GdkPixbuf ** gp, Pixmap * p);
static gboolean read_int(XrmDatabase db, const gchar *rname, gint *value);
static gboolean read_string(XrmDatabase db, const gchar *rname, gchar **value);
static gboolean read_color(XrmDatabase db, const RrInstance *inst,
        const gchar *rname, RrColor **value);
static gboolean read_mask(const RrInstance *inst, const gchar *path,
        ObFrameThemeConfig *theme, const gchar *maskname, RrPixmapMask **value);
static gboolean read_appearance(XrmDatabase db, const RrInstance *inst,
        const gchar *rname, RrAppearance *value, gboolean allow_trans);
static int parse_inline_number(const char *p);
static RrPixel32* read_c_image(gint width, gint height, const guint8 *data);
static void set_default_appearance(RrAppearance *a);

gint load_theme_config(const RrInstance *inst, const gchar *name,
        const gchar * path, XrmDatabase db, RrFont *active_window_font,
        RrFont *inactive_window_font, RrFont *menu_title_font,
        RrFont *menu_item_font, RrFont *osd_font)
{
    gchar *str;

    if (!read_int(db, "concept.border_width", &theme_config.border_width)) {
        theme_config.border_width = 2;
    }
    if (!read_color(db, inst, "concept.focus_border_color",
            &theme_config.focus_border_color)) {
        theme_config.focus_border_color = RrColorNew(inst, 0, 0, 0);
    }

    if (!read_color(db, inst, "concept.focus_corner_color",
            &theme_config.focus_corner_color)) {
        theme_config.focus_corner_color = RrColorNew(inst, 0, 0, 0);
    }

    if (!read_color(db, inst, "concept.unfocus_border_color",
            &theme_config.unfocus_border_color)) {
        theme_config.focus_border_color = RrColorNew(inst, 0, 0, 0);
    }

    if (!read_color(db, inst, "concept.unfocus_corner_color",
            &theme_config.unfocus_corner_color)) {
        theme_config.focus_corner_color = RrColorNew(inst, 0, 0, 0);
    }

    theme_config.inst = inst;
    theme_config.name = g_strdup(name ? name : DEFAULT_THEME);

    gdk_pixbuf_xlib_init(plugin.ob_display, plugin.ob_screen);

    load_pixmap(name, "focus-top.png", &theme_config.focus_top,
            &theme_config.px_focus_top);
    load_pixmap(name, "focus-left.png", &theme_config.focus_left,
            &theme_config.px_focus_left);
    load_pixmap(name, "focus-right.png", &theme_config.focus_right,
            &theme_config.px_focus_right);
    load_pixmap(name, "focus-bottom.png", &theme_config.focus_bottom,
            &theme_config.px_focus_bottom);

    load_pixmap(name, "unfocus-top.png", &theme_config.unfocus_top,
            &theme_config.px_unfocus_top);
    load_pixmap(name, "unfocus-left.png", &theme_config.unfocus_left,
            &theme_config.px_unfocus_left);
    load_pixmap(name, "unfocus-right.png", &theme_config.unfocus_right,
            &theme_config.px_unfocus_right);
    load_pixmap(name, "unfocus-bottom.png", &theme_config.unfocus_bottom,
            &theme_config.px_unfocus_bottom);

    load_pixmap(name, "focus-topleft.png", &theme_config.focus_topleft,
            &theme_config.px_focus_topleft);
    load_pixmap(name, "focus-bottomleft.png", &theme_config.focus_bottomleft,
            &theme_config.px_focus_bottomleft);
    load_pixmap(name, "focus-topright.png", &theme_config.focus_topright,
            &theme_config.px_focus_topright);
    load_pixmap(name, "focus-bottomright.png", &theme_config.focus_bottomright,
            &theme_config.px_focus_bottomright);

    load_pixmap(name, "unfocus-topleft.png", &theme_config.unfocus_topleft,
            &theme_config.px_unfocus_topleft);
    load_pixmap(name, "unfocus-bottomleft.png",
            &theme_config.unfocus_bottomleft,
            &theme_config.px_unfocus_bottomleft);
    load_pixmap(name, "unfocus-topright.png", &theme_config.unfocus_topright,
            &theme_config.px_unfocus_topright);
    load_pixmap(name, "unfocus-bottomright.png",
            &theme_config.unfocus_bottomright,
            &theme_config.px_unfocus_bottomright);

    return 1;
}

void load_pixmap(const gchar * theme_name, const gchar * base_name,
        GdkPixbuf ** gp, Pixmap * p)
{
    gchar * s = g_build_filename(g_get_home_dir(), ".themes", theme_name,
            "openbox-3", base_name, NULL);
    ob_debug("Load file %s.\n", s);
    *gp = gdk_pixbuf_new_from_file(s, NULL);
    //GdkPixbuff * mask = NULL;
    gdk_pixbuf_xlib_render_pixmap_and_mask(*gp, p, NULL, 128);
    g_free(s);
}

static gchar *create_class_name(const gchar *rname)
{
    gchar *rclass = g_strdup(rname);
    gchar *p = rclass;

    while (TRUE) {
        *p = toupper(*p);
        p = strchr(p+1, '.');
        if (p == NULL)
            break;
        ++p;
        if (*p == '\0')
            break;
    }
    return rclass;
}

static gboolean read_int(XrmDatabase db, const gchar *rname, gint *value)
{
    gboolean ret = FALSE;
    gchar *rclass = create_class_name(rname);
    gchar *rettype, *end;
    XrmValue retvalue;

    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) && retvalue.addr
            != NULL) {
        *value = (gint)strtol(retvalue.addr, &end, 10);
        if (end != retvalue.addr)
            ret = TRUE;
    }

    g_free(rclass);
    return ret;
}

static gboolean read_string(XrmDatabase db, const gchar *rname, gchar **value)
{
    gboolean ret = FALSE;
    gchar *rclass = create_class_name(rname);
    gchar *rettype;
    XrmValue retvalue;

    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) && retvalue.addr
            != NULL) {
        *value = retvalue.addr;
        ret = TRUE;
    }

    g_free(rclass);
    return ret;
}

static gboolean read_color(XrmDatabase db, const RrInstance *inst,
        const gchar *rname, RrColor **value)
{
    gboolean ret = FALSE;
    gchar *rclass = create_class_name(rname);
    gchar *rettype;
    XrmValue retvalue;

    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) && retvalue.addr
            != NULL) {
        RrColor *c = RrColorParse(inst, retvalue.addr);
        if (c != NULL) {
            *value = c;
            ret = TRUE;
        }
    }

    g_free(rclass);
    return ret;
}

static gboolean read_mask(const RrInstance *inst, const gchar *path,
        ObFrameThemeConfig *theme, const gchar *maskname, RrPixmapMask **value)
{
    gboolean ret = FALSE;
    gchar *s;
    gint hx, hy; /* ignored */
    guint w, h;
    guchar *b;

    s = g_build_filename(path, maskname, NULL);
    if (XReadBitmapFileData(s, &w, &h, &b, &hx, &hy) == BitmapSuccess) {
        ret = TRUE;
        *value = RrPixmapMaskNew(inst, w, h, (gchar*)b);
        XFree(b);
    }
    g_free(s);

    return ret;
}

static void parse_appearance(gchar *tex, RrSurfaceColorType *grad,
        RrReliefType *relief, RrBevelType *bevel, gboolean *interlaced,
        gboolean *border, gboolean allow_trans)
{
    gchar *t;

    /* convert to all lowercase */
    for (t = tex; *t != '\0'; ++t)
        *t = g_ascii_tolower(*t);

    if (allow_trans && strstr(tex, "parentrelative") != NULL) {
        *grad = RR_SURFACE_PARENTREL;
    }
    else {
        if (strstr(tex, "gradient") != NULL) {
            if (strstr(tex, "crossdiagonal") != NULL)
                *grad = RR_SURFACE_CROSS_DIAGONAL;
            else if (strstr(tex, "pyramid") != NULL)
                *grad = RR_SURFACE_PYRAMID;
            else if (strstr(tex, "mirrorhorizontal") != NULL)
                *grad = RR_SURFACE_MIRROR_HORIZONTAL;
            else if (strstr(tex, "horizontal") != NULL)
                *grad = RR_SURFACE_HORIZONTAL;
            else if (strstr(tex, "splitvertical") != NULL)
                *grad = RR_SURFACE_SPLIT_VERTICAL;
            else if (strstr(tex, "vertical") != NULL)
                *grad = RR_SURFACE_VERTICAL;
            else
                *grad = RR_SURFACE_DIAGONAL;
        }
        else {
            *grad = RR_SURFACE_SOLID;
        }
    }

    if (strstr(tex, "sunken") != NULL)
        *relief = RR_RELIEF_SUNKEN;
    else if (strstr(tex, "flat") != NULL)
        *relief = RR_RELIEF_FLAT;
    else if (strstr(tex, "raised") != NULL)
        *relief = RR_RELIEF_RAISED;
    else
        *relief = (*grad == RR_SURFACE_PARENTREL) ? RR_RELIEF_FLAT
                : RR_RELIEF_RAISED;

    *border = FALSE;
    if (*relief == RR_RELIEF_FLAT) {
        if (strstr(tex, "border") != NULL)
            *border = TRUE;
    }
    else {
        if (strstr(tex, "bevel2") != NULL)
            *bevel = RR_BEVEL_2;
        else
            *bevel = RR_BEVEL_1;
    }

    if (strstr(tex, "interlaced") != NULL)
        *interlaced = TRUE;
    else
        *interlaced = FALSE;
}

static gboolean read_appearance(XrmDatabase db, const RrInstance *inst,
        const gchar *rname, RrAppearance *value, gboolean allow_trans)
{
    gboolean ret = FALSE;
    gchar *rclass = create_class_name(rname);
    gchar *cname, *ctoname, *bcname, *icname, *hname, *sname;
    gchar *csplitname, *ctosplitname;
    gchar *rettype;
    XrmValue retvalue;
    gint i;

    cname = g_strconcat(rname, ".color", NULL);
    ctoname = g_strconcat(rname, ".colorTo", NULL);
    bcname = g_strconcat(rname, ".border.color", NULL);
    icname = g_strconcat(rname, ".interlace.color", NULL);
    hname = g_strconcat(rname, ".highlight", NULL);
    sname = g_strconcat(rname, ".shadow", NULL);
    csplitname = g_strconcat(rname, ".color.splitTo", NULL);
    ctosplitname = g_strconcat(rname, ".colorTo.splitTo", NULL);

    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) && retvalue.addr
            != NULL) {
        parse_appearance(retvalue.addr, &value->surface.grad,
                &value->surface.relief, &value->surface.bevel,
                &value->surface.interlaced, &value->surface.border, allow_trans);
        if (!read_color(db, inst, cname, &value->surface.primary))
            value->surface.primary = RrColorNew(inst, 0, 0, 0);
        if (!read_color(db, inst, ctoname, &value->surface.secondary))
            value->surface.secondary = RrColorNew(inst, 0, 0, 0);
        if (value->surface.border)
            if (!read_color(db, inst, bcname, &value->surface.border_color))
                value->surface.border_color = RrColorNew(inst, 0, 0, 0);
        if (value->surface.interlaced)
            if (!read_color(db, inst, icname, &value->surface.interlace_color))
                value->surface.interlace_color = RrColorNew(inst, 0, 0, 0);
        if (read_int(db, hname, &i) && i >= 0)
            value->surface.bevel_light_adjust = i;
        if (read_int(db, sname, &i) && i >= 0 && i <= 256)
            value->surface.bevel_dark_adjust = i;

        if (value->surface.grad == RR_SURFACE_SPLIT_VERTICAL) {
            gint r, g, b;

            if (!read_color(db, inst, csplitname, &value->surface.split_primary)) {
                r = value->surface.primary->r;
                r += r >> 2;
                g = value->surface.primary->g;
                g += g >> 2;
                b = value->surface.primary->b;
                b += b >> 2;
                if (r > 0xFF)
                    r = 0xFF;
                if (g > 0xFF)
                    g = 0xFF;
                if (b > 0xFF)
                    b = 0xFF;
                value->surface.split_primary = RrColorNew(inst, r, g, b);
            }

            if (!read_color(db, inst, ctosplitname,
                    &value->surface.split_secondary)) {
                r = value->surface.secondary->r;
                r += r >> 4;
                g = value->surface.secondary->g;
                g += g >> 4;
                b = value->surface.secondary->b;
                b += b >> 4;
                if (r > 0xFF)
                    r = 0xFF;
                if (g > 0xFF)
                    g = 0xFF;
                if (b > 0xFF)
                    b = 0xFF;
                value->surface.split_secondary = RrColorNew(inst, r, g, b);
            }
        }

        ret = TRUE;
    }

    g_free(ctosplitname);
    g_free(csplitname);
    g_free(sname);
    g_free(hname);
    g_free(icname);
    g_free(bcname);
    g_free(ctoname);
    g_free(cname);
    g_free(rclass);
    return ret;
}

static int parse_inline_number(const char *p)
{
    int neg = 1;
    int res = 0;
    if (*p == '-') {
        neg = -1;
        ++p;
    }
    for (; isdigit(*p); ++p)
        res = res * 10 + *p - '0';
    res *= neg;
    return res;
}

static void set_default_appearance(RrAppearance *a)
{
    a->surface.grad = RR_SURFACE_SOLID;
    a->surface.relief = RR_RELIEF_FLAT;
    a->surface.bevel = RR_BEVEL_1;
    a->surface.interlaced = FALSE;
    a->surface.border = FALSE;
    a->surface.primary = RrColorNew(a->inst, 0, 0, 0);
    a->surface.secondary = RrColorNew(a->inst, 0, 0, 0);
}

/* Reads the output from gimp's C-Source file format into valid RGBA data for
 an RrTextureRGBA. */
static RrPixel32* read_c_image(gint width, gint height, const guint8 *data)
{
    RrPixel32 *im, *p;
    gint i;

    p = im = g_memdup(data, width * height * sizeof(RrPixel32));

    for (i = 0; i < width * height; ++i) {
        guchar a = ((*p >> 24) & 0xff);
        guchar b = ((*p >> 16) & 0xff);
        guchar g = ((*p >> 8) & 0xff);
        guchar r = ((*p >> 0) & 0xff);

        *p = ((r << RrDefaultRedOffset) + (g << RrDefaultGreenOffset) + (b
                << RrDefaultBlueOffset) + (a << RrDefaultAlphaOffset));
        p++;
    }

    return im;
}
