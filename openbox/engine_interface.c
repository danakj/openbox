/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

 keyboard.c for the Openbox window manager
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

#include "engine_interface.h"
#include "config.h"
#include "debug.h"
#include "openbox.h"
#include "obt/paths.h"

#define SHARED_SUFFIX ".la"

gchar *create_class_name(const gchar *rname);

/* Load XrmDatabase */
XrmDatabase loaddb(const gchar *name, gchar **path);

/* Read string in XrmDatabase */
gboolean read_string(XrmDatabase db, const gchar *rname, gchar **value);

void init_frame_engine(ObFrameEngine * p_engine, const gchar *name, gboolean allow_fallback,
        RrFont *active_window_font, RrFont *inactive_window_font,
        RrFont *menu_title_font, RrFont *menu_item_font, RrFont *osd_font)
{
    XrmDatabase db = NULL;
    gchar *path;

    if (name) {
        db = loaddb(name, &path);
        if (db == NULL) {
            g_message("Unable to load the theme '%s'", name);
        }
    }

    gchar * engine_filename;
    if (!read_string(db, "frame.theme.engine", &engine_filename)) {
        engine_filename = "libdefault.la";
    }
    ob_debug("Try to init : %s", engine_filename);
    gchar * absolute_engine_filename = g_build_filename(g_get_home_dir(),
            ".config", "openbox", "engines", engine_filename, NULL);
    ObFrameEngine * p = load_frame_engine(absolute_engine_filename);
    g_free(absolute_engine_filename);

    memcpy(p_engine, p, sizeof(ObFrameEngine));
    update_frame_engine();

    (p->load_theme_config)(ob_rr_inst, name, path, db, active_window_font,
            inactive_window_font, menu_title_font, menu_item_font, osd_font);

    g_free(path);
    XrmDestroyDatabase(db);
}

void update_frame_engine()
{
    frame_engine.init (obt_display, ob_screen);
}

ObFrameEngine * load_frame_engine(const gchar * filename)
{
    GModule *module;
    gpointer func;

    if (!(module = g_module_open(filename, G_MODULE_BIND_LOCAL))) {
        ob_debug ("Failed to load engine (%s): %s\n",
                filename, g_module_error());
        exit(1);
    }

    if (g_module_symbol(module, "get_info", &func)) {
        ObFrameEngine *engine = (ObFrameEngine *) ((ObFrameEngineFunc) func)();
        return engine;
    }
    else {
        ob_debug_type(OB_DEBUG_SM,
                "Failed to get \"get_info\" function (%s): %s\n", filename,
                g_module_error());
        exit(1);
    }

    ob_debug_type(OB_DEBUG_SM, "Invalid engine (%s)\n", filename);
    g_module_close(module);
    exit(1);
}

gboolean read_string(XrmDatabase db, const gchar *rname, gchar **value)
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

XrmDatabase loaddb(const gchar *name, gchar **path)
{
    GSList *it;
    XrmDatabase db = NULL;
    gchar *s;

    if (name[0] == '/') {
        s = g_build_filename(name, "openbox-3", "themerc", NULL);
        if ((db = XrmGetFileDatabase(s)))
            *path = g_path_get_dirname(s);
        g_free(s);
    }
    else {
        ObtPaths *p;

        p = obt_paths_new();
        /* XXX backwards compatibility, remove me sometime later */
        s = g_build_filename(g_get_home_dir(), ".themes", name, "openbox-3",
                "themerc", NULL);
        if ((db = XrmGetFileDatabase(s)))
            *path = g_path_get_dirname(s);
        g_free(s);

        for (it = obt_paths_data_dirs(p); !db && it; it = g_slist_next(it)) {
            s = g_build_filename(it->data, "themes", name, "openbox-3",
                    "themerc", NULL);
            if ((db = XrmGetFileDatabase(s)))
                *path = g_path_get_dirname(s);
            g_free(s);
        }
    }

    if (db == NULL) {
        s = g_build_filename(name, "themerc", NULL);
        if ((db = XrmGetFileDatabase(s)))
            *path = g_path_get_dirname(s);
        g_free(s);
    }

    return db;
}

gchar *create_class_name(const gchar *rname)
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

ObFrameContext frame_context_from_string(const gchar *name)
{
    if (!g_ascii_strcasecmp("Desktop", name))
        return OB_FRAME_CONTEXT_DESKTOP;
    else if (!g_ascii_strcasecmp("Root", name))
        return OB_FRAME_CONTEXT_ROOT;
    else if (!g_ascii_strcasecmp("Client", name))
        return OB_FRAME_CONTEXT_CLIENT;
    else if (!g_ascii_strcasecmp("Titlebar", name))
        return OB_FRAME_CONTEXT_TITLEBAR;
    else if (!g_ascii_strcasecmp("Frame", name))
        return OB_FRAME_CONTEXT_FRAME;
    else if (!g_ascii_strcasecmp("TLCorner", name))
        return OB_FRAME_CONTEXT_TLCORNER;
    else if (!g_ascii_strcasecmp("TRCorner", name))
        return OB_FRAME_CONTEXT_TRCORNER;
    else if (!g_ascii_strcasecmp("BLCorner", name))
        return OB_FRAME_CONTEXT_BLCORNER;
    else if (!g_ascii_strcasecmp("BRCorner", name))
        return OB_FRAME_CONTEXT_BRCORNER;
    else if (!g_ascii_strcasecmp("Top", name))
        return OB_FRAME_CONTEXT_TOP;
    else if (!g_ascii_strcasecmp("Bottom", name))
        return OB_FRAME_CONTEXT_BOTTOM;
    else if (!g_ascii_strcasecmp("Handle", name))
        return OB_FRAME_CONTEXT_BOTTOM;
    else if (!g_ascii_strcasecmp("Left", name))
        return OB_FRAME_CONTEXT_LEFT;
    else if (!g_ascii_strcasecmp("Right", name))
        return OB_FRAME_CONTEXT_RIGHT;
    else if (!g_ascii_strcasecmp("Maximize", name))
        return OB_FRAME_CONTEXT_MAXIMIZE;
    else if (!g_ascii_strcasecmp("AllDesktops", name))
        return OB_FRAME_CONTEXT_ALLDESKTOPS;
    else if (!g_ascii_strcasecmp("Shade", name))
        return OB_FRAME_CONTEXT_SHADE;
    else if (!g_ascii_strcasecmp("Iconify", name))
        return OB_FRAME_CONTEXT_ICONIFY;
    else if (!g_ascii_strcasecmp("Icon", name))
        return OB_FRAME_CONTEXT_ICON;
    else if (!g_ascii_strcasecmp("Close", name))
        return OB_FRAME_CONTEXT_CLOSE;
    else if (!g_ascii_strcasecmp("MoveResize", name))
        return OB_FRAME_CONTEXT_MOVE_RESIZE;
    return OB_FRAME_CONTEXT_NONE;
}

ObFrameContext engine_frame_context(ObClient *client, Window win, gint x, gint y)
{
    /* this part is commun to all engine */
    if (moveresize_in_progress)
        return OB_FRAME_CONTEXT_MOVE_RESIZE;
    if (win == obt_root(ob_screen))
        return OB_FRAME_CONTEXT_ROOT;
    if (client == NULL)
        return OB_FRAME_CONTEXT_NONE;
    if (win == client->w_client) {
        /* conceptually, this is the desktop, as far as users are
         concerned */
        if (client->type == OB_CLIENT_TYPE_DESKTOP)
            return OB_FRAME_CONTEXT_DESKTOP;
        return OB_FRAME_CONTEXT_CLIENT;
    }
    /* this part is specific to the plugin */
    return frame_engine.frame_context(client->frame, win, x, y);

}

void frame_client_gravity(ObClient *self, gint *x, gint *y)
{
    Strut size;
    frame_engine.frame_get_size(self->frame, &size);
    /* horizontal */
    switch (self->gravity) {
    default:
    case NorthWestGravity:
    case SouthWestGravity:
    case WestGravity:
        break;

    case NorthGravity:
    case SouthGravity:
    case CenterGravity:
        /* the middle of the client will be the middle of the frame */
        *x -= (size.right - size.left) / 2;
        break;

    case NorthEastGravity:
    case SouthEastGravity:
    case EastGravity:
        /* the right side of the client will be the right side of the frame */
        *x -= size.right + size.left - self->border_width * 2;
        break;

    case ForgetGravity:
    case StaticGravity:
        /* the client's position won't move */
        *x -= size.left - self->border_width;
        break;
    }

    /* vertical */
    switch (self->gravity) {
    default:
    case NorthWestGravity:
    case NorthEastGravity:
    case NorthGravity:
        break;

    case CenterGravity:
    case EastGravity:
    case WestGravity:
        /* the middle of the client will be the middle of the frame */
        *y -= (size.bottom - size.top) / 2;
        break;

    case SouthWestGravity:
    case SouthEastGravity:
    case SouthGravity:
        /* the bottom of the client will be the bottom of the frame */
        *y -= size.bottom + size.top - self->border_width * 2;
        break;

    case ForgetGravity:
    case StaticGravity:
        /* the client's position won't move */
        *y -= size.top - self->border_width;
        break;
    }
}

void frame_frame_gravity(ObClient *self, gint *x, gint *y)
{
    Strut size;
    frame_engine.frame_get_size(self->frame, &size);
    /* horizontal */
    switch (self->gravity) {
    default:
    case NorthWestGravity:
    case WestGravity:
    case SouthWestGravity:
        break;
    case NorthGravity:
    case CenterGravity:
    case SouthGravity:
        /* the middle of the client will be the middle of the frame */
        *x += (size.right - size.left) / 2;
        break;
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
        /* the right side of the client will be the right side of the frame */
        *x += size.right + size.left - self->border_width * 2;
        break;
    case StaticGravity:
    case ForgetGravity:
        /* the client's position won't move */
        *x += size.left - self->border_width;
        break;
    }

    /* vertical */
    switch (self->gravity) {
    default:
    case NorthWestGravity:
    case NorthGravity:
    case NorthEastGravity:
        break;
    case WestGravity:
    case CenterGravity:
    case EastGravity:
        /* the middle of the client will be the middle of the frame */
        *y += (size.bottom - size.top) / 2;
        break;
    case SouthWestGravity:
    case SouthGravity:
    case SouthEastGravity:
        /* the bottom of the client will be the bottom of the frame */
        *y += size.bottom + size.top - self->border_width * 2;
        break;
    case StaticGravity:
    case ForgetGravity:
        /* the client's position won't move */
        *y += size.top - self->border_width;
        break;
    }
}

void frame_rect_to_frame(ObClient * self, Rect *r)
{
    Strut size;
    frame_engine.frame_get_size(self->frame, &size);
    r->width += size.left + size.right;
    r->height += size.top + size.bottom;
    frame_client_gravity(self, &r->x, &r->y);
}

void frame_rect_to_client(ObClient * self, Rect *r)
{
    Strut size;
    frame_engine.frame_get_size(self, &size);
    r->width -= size.left + size.right;
    r->height -= size.top + size.bottom;
    frame_frame_gravity(self, &r->x, &r->y);
}
