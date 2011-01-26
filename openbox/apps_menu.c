/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   apps_menu.c for the Openbox window manager
   Copyright (c) 2011        Dana Jansens

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

#include "openbox.h"
#include "menu.h"
#include "menuframe.h"
#include "screen.h"
#include "apps_menu.h"
#include "config.h"
#include "gettext.h"
#include "obt/linkbase.h"
#include "obt/link.h"
#include "obt/paths.h"

#include <glib.h>

#define MENU_NAME "apps-menu"

typedef struct _ObAppsMenuCategory ObAppsMenuCategory;

struct _ObAppsMenuCategory {
    GQuark q;
    gchar *friendly;
    ObMenu *m;
};

static ObMenu *apps_menu;
static ObtLinkBase *linkbase;
static gboolean dirty;

static ObAppsMenuCategory *categories;
/*! An array of pointers to the categories array, sorted by their friendly
  names. */
static ObAppsMenuCategory **sorted_categories;
static guint n_categories;

static void self_destroy(ObMenu *menu, gpointer data)
{
    menu_clear_entries(menu);
}

static gboolean self_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    ObMenuEntry *e;
    guint i;

    g_print("UPDATE DIRTY %d\n", dirty);

    if (!dirty) return TRUE;  /* nothing has changed to be updated */

    menu_clear_entries(menu);

    for (i = 0; i < n_categories; ++i) {
        menu_free(categories[i].m);
        categories[i].m = NULL;
    }


    for (i = 0; i < n_categories; ++i) {
        // XXX if not empty
        ObAppsMenuCategory *cat = sorted_categories[i];
        const gchar *label = g_quark_to_string(cat->q);
        if (!cat->m)
            cat->m = menu_new(label, cat->friendly, FALSE, NULL);
        e = menu_add_submenu(menu, i, label);
    }
/*
    menu_add_separator(menu, SEPARATOR, screen_desktop_names[desktop]);

    gchar *title = g_strdup_printf("(%s)", c->icon_title);
    e = menu_add_normal(menu, desktop, title, NULL, FALSE);
    g_free(title);

    if (config_menu_show_icons) {
        e->data.normal.icon = client_icon(c);
        RrImageRef(e->data.normal.icon);
        e->data.normal.icon_alpha =
            c->iconic ? OB_ICONIC_ALPHA : 0xff;
    }

    e->data.normal.data = link;
*/


    dirty = FALSE;
    return TRUE; /* always show the menu */
}

static void menu_execute(ObMenuEntry *self, ObMenuFrame *f,
                         ObClient *c, guint state, gpointer data)
{
#if 0
    ObClient *t = self->data.normal.data;
    if (t) { /* it's set to NULL if its destroyed */
        gboolean here = state & ShiftMask;

        client_activate(t, TRUE, here, TRUE, TRUE, TRUE);
        /* if the window is omnipresent then we need to go to its
           desktop */
        if (!here && t->desktop == DESKTOP_ALL)
            screen_set_desktop(self->id, FALSE);
    }
#endif
}

static void linkbase_update(ObtLinkBase *lb, gpointer data)
{
    dirty = TRUE;
}

static int cat_cmp(const void *a, const void *b)
{
    const ObAppsMenuCategory *ca = a, *cb = b;
    return ca->q - cb->q;
}

static int cat_friendly_cmp(const void *a, const void *b)
{
    ObAppsMenuCategory *const *ca = a, *const *cb = b;
    return strcmp((*ca)->friendly, (*cb)->friendly);
}

void apps_menu_startup(gboolean reconfig)
{
    if (!reconfig) {
        ObtPaths *paths;
        guint i;

        paths = obt_paths_new();
        /* XXX allow more environments, like GNOME or KDE, to be included */
        linkbase = obt_linkbase_new(paths, ob_locale_msg,
                                    OBT_LINK_ENV_OPENBOX);
        obt_paths_unref(paths);
        obt_linkbase_set_update_func(linkbase, linkbase_update, NULL);

        dirty = TRUE;

        /* From http://standards.freedesktop.org/menu-spec/latest/apa.html */
        n_categories = 10;
        categories = g_new0(ObAppsMenuCategory, n_categories);
        sorted_categories = g_new(ObAppsMenuCategory*, n_categories);

        categories[0].q = g_quark_from_static_string("AudioVideo");
        categories[0].friendly = _("Sound & Video");
        categories[1].q = g_quark_from_static_string("Development");
        categories[1].friendly = _("Programming");
        categories[2].q = g_quark_from_static_string("Education");
        categories[2].friendly = _("Education");
        categories[3].q = g_quark_from_static_string("Game");
        categories[3].friendly = _("Games");
        categories[4].q = g_quark_from_static_string("Graphics");
        categories[4].friendly = _("Graphics");
        categories[5].q = g_quark_from_static_string("Network");
        categories[5].friendly = _("Internet");
        categories[6].q = g_quark_from_static_string("Office");
        categories[6].friendly = _("Office");
        categories[7].q = g_quark_from_static_string("Settings");
        categories[7].friendly = _("Settings");
        categories[8].q = g_quark_from_static_string("System");
        categories[8].friendly = _("System");
        categories[9].q = g_quark_from_static_string("Utility");
        categories[9].friendly = _("Utility");
        /* Sort them by their quark values */
        qsort(categories, n_categories, sizeof(ObAppsMenuCategory), cat_cmp);

        for (i = 0; i < n_categories; ++i)
            sorted_categories[i] = &categories[i];
        qsort(sorted_categories, n_categories, sizeof(void*),
              cat_friendly_cmp);
    }

    apps_menu = menu_new(MENU_NAME, _("Applications"), TRUE, NULL);
    menu_set_update_func(apps_menu, self_update);
    menu_set_destroy_func(apps_menu, self_destroy);
    menu_set_execute_func(apps_menu, menu_execute);
}

void apps_menu_shutdown(gboolean reconfig)
{
    if (!reconfig) {
        guint i;

        obt_linkbase_unref(linkbase);
        linkbase = NULL;

        for (i = 0; i < n_categories; ++i)
            if (categories[i].m)
                menu_free(categories[i].m);
        g_free(categories);
        categories = NULL;
        g_free(sorted_categories);
        sorted_categories = NULL;
    }

    /* freed by the hash table */
}
