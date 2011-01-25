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

static ObMenu *apps_menu;
static ObtLinkBase *linkbase;
static gboolean dirty;

static void self_cleanup(ObMenu *menu, gpointer data)
{
    menu_clear_entries(menu);
}

static gboolean self_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    ObMenuEntry *e;

    if (!dirty) return TRUE;  /* nothing has changed to be updated */

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

void linkbase_update(ObtLinkBase *lb, gpointer data)
{
    dirty = TRUE;
}

void apps_menu_startup(gboolean reconfig)
{
    if (!reconfig) {
        ObtPaths *paths;

        paths = obt_paths_new();
        /* XXX allow more environments, like GNOME or KDE, to be included */
        linkbase = obt_linkbase_new(paths, ob_locale_msg,
                                    OBT_LINK_ENV_OPENBOX);
        obt_paths_unref(paths);
    }

    apps_menu = menu_new(MENU_NAME, _("Applications"), TRUE, NULL);
    menu_set_update_func(apps_menu, self_update);
    menu_set_cleanup_func(apps_menu, self_cleanup);
    menu_set_execute_func(apps_menu, menu_execute);
}

void apps_menu_shutdown(gboolean reconfig)
{
    if (!reconfig) {
        obt_linkbase_unref(linkbase);
        linkbase = NULL;
    }

    /* freed by the hash table */
}
