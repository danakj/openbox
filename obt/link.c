/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/link.c for the Openbox window manager
   Copyright (c) 2009        Dana Jansens

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

#include "obt/link.h"
#include "obt/ddparse.h"
#include <glib.h>

struct _ObtLink {
    guint ref;

    ObtLinkType type;
    gchar *name; /*!< Specific name for the object (eg Firefox) */
    gchar *generic; /*!< Generic name for the object (eg Web Browser) */
    gchar *comment; /*!< Comment/description to display for the object */
    gchar *icon; /*!< Name/path for an icon for the object */

    union _ObtLinkData {
        struct _ObtLinkApp {
            gchar *exec; /*!< Executable to run for the app */
            gchar *wdir; /*!< Working dir to run the app in */
            gboolean term; /*!< Run the app in a terminal or not */
            ObtLinkAppOpen open;

            /* XXX gchar**? or something better, a mime struct.. maybe
               glib has something i can use. */
            gchar **mime; /*!< Mime types the app can open */

            ObtLinkAppStartup startup;
            gchar *startup_wmclass;
        } app;
        struct _ObtLinkLink {
            gchar *url;
        } link;
        struct _ObtLinkDir {
        } dir;
    } d;
};

ObtLink* obt_link_from_ddfile(const gchar *name, GSList *paths)
{
    ObtLink *lnk;
    GHashTable *groups, *keys;
    ObtDDParseGroup *g;

    groups = obt_ddparse_file(name, paths);
    if (!groups) return NULL;
    g = g_hash_table_lookup(groups, "Desktop Entry");
    if (!g) {
        g_hash_table_destroy(groups);
        return NULL;
    }

    keys = obt_ddparse_group_keys(g);

    lnk = g_slice_new(ObtLink);
    lnk->ref = 1;
    /* XXX turn the values in the .desktop file into an ObtLink */

    return lnk;
}

void obt_link_ref(ObtLink *dd)
{
    ++dd->ref;
}

void obt_link_unref(ObtLink *dd)
{
    if (--dd->ref < 1) {
        g_slice_free(ObtLink, dd);
    }
}
