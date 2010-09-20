/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-
 
   obt/linkbase.c for the Openbox window manager
   Copyright (c) 2010        Dana Jansens
 
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

#include "obt/linkbase.h"
#include "obt/link.h"
#include "obt/paths.h"
#include "obt/watch.h"

struct _ObtLinkBase {
    gint ref;

    ObtWatch *watch;
    GHashTable *base;
};

static void func(ObtWatch *w, const gchar *subpath, ObtWatchNotifyType type,
                 gpointer data)
{
}

ObtLinkBase* obt_linkbase_new(ObtPaths *paths)
{
    ObtLinkBase *b;
    GSList *it;
    
    b = g_slice_new(ObtLinkBase);
    b->watch = obt_watch_new();
    b->base = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                    (GDestroyNotify)obt_link_unref);

    for (it = obt_paths_data_dirs(paths); it; it = g_slist_next(it)) {
        gchar *p;
        p = g_strconcat(it->data, "/applications", NULL);
        obt_watch_add(b->watch, p, FALSE, func, b);
    }
    return b;
}

void obt_linkbase_ref(ObtLinkBase *lb)
{
    ++lb->ref;
}

void obt_linkbase_unref(ObtLinkBase *lb)
{
    if (--lb->ref < 1) {
        obt_watch_unref(lb->watch);
        g_hash_table_unref(lb->base);
        g_slice_free(ObtLinkBase, lb);
    }
}
