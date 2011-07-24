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

#ifdef HAVE_STRING_H
# include <string.h>
#endif

typedef struct _ObtLinkBaseEntry    ObtLinkBaseEntry;
typedef struct _ObtLinkBaseCategory ObtLinkBaseCategory;

struct _ObtLinkBaseEntry {
    /*! Links come from a set of paths.  Links found in earlier paths get lower
      priority values (higher precedence).  This is the index in that set of
      paths of the base directory under which the link was found. */
    gint priority;
    ObtLink *link;
};

struct _ObtLinkBaseCategory {
    GQuark cat;
    GSList *links;
};

struct _ObtLinkBase {
    gint ref;

    /*! A bitflag of values from ObtLinkEnvFlags indicating which environments
      are to be considered active. */
    guint environments;

    const gchar *language;
    const gchar *country;
    const gchar *modifier;

    ObtPaths *paths;
    ObtWatch *watch;
    /*! This holds a GList of ObtLinkBaseEntrys sorted by priority in
      increasing order (by precedence in decreasing order). */
    GHashTable *base;
    /*! This holds the paths in which we look for links, and the data is an
      integer that is the priority of that directory. */
    GHashTable *path_to_priority;

    /*! This maps GQuark main categories to ObtLinkBaseCategory objects,
      containing lists of ObtLink objects found in
      the category. The ObtLink objects are not reffed to be placed in this
      structure since they will always be in the base hash table as well. So
      they are not unreffed when they are removed. */
    GHashTable *categories;

    ObtLinkBaseUpdateFunc update_func;
    gpointer update_data;
};

static void base_entry_free(ObtLinkBaseEntry *e)
{
    obt_link_unref(e->link);
    g_slice_free(ObtLinkBaseEntry, e);
}

static void base_entry_list_free(gpointer key, gpointer value, gpointer data)
{
    GList *it, *list; (void)key; (void)data;

    list = value;
    for (it = list; it; it = g_list_next(it))
        base_entry_free(it->data);
    g_list_free(list);
}

static GList* find_base_entry_path(GList *list, const gchar *full_path)
{
    GList *it;
    for (it = list; it; it = g_list_next(it)) {
        ObtLinkBaseEntry *e = it->data;
        if (strcmp(obt_link_source_file(e->link), full_path) == 0)
            break;
    }
    return it;
}

/*! Finds the first entry in the list with a priority number >= @priority. */
static GList* find_base_entry_priority(GList *list, gint priority)
{
    GList *it;
    for (it = list; it; it = g_list_next(it)) {
        ObtLinkBaseEntry *e = it->data;
        if (e->priority >= priority)
            break;
    }
    return it;
}

static void category_add(ObtLinkBase *lb, GQuark cat, ObtLink *link)
{
    ObtLinkBaseCategory *lc;

    lc = g_hash_table_lookup(lb->categories, &cat);
    if (!lc) {
        lc = g_slice_new(ObtLinkBaseCategory);
        lc->cat = cat;
        lc->links = NULL;
        g_hash_table_insert(lb->categories, &lc->cat, lc);
    }
    lc->links = g_slist_prepend(lc->links, link);
}

static void category_remove(ObtLinkBase *lb, GQuark cat, ObtLink *link)
{
    ObtLinkBaseCategory *lc;
    GSList *it;

    lc = g_hash_table_lookup(lb->categories, &cat);

    it = lc->links;
    while (it->data != link)
        it = g_slist_next(it);
    lc->links = g_slist_delete_link(lc->links, it);
    if (!lc->links)
        g_hash_table_remove(lb->categories, &cat);
}

static void category_add_app(ObtLinkBase *lb, ObtLink *link)
{
    if (obt_link_type(link) == OBT_LINK_TYPE_APPLICATION) {
        const GQuark *cats;
        gulong i, n;

        cats = obt_link_app_categories(link, &n);
        for (i = 0; i < n; ++i)
            category_add(lb, cats[i], link);
    }
}

static void category_remove_app(ObtLinkBase *lb, ObtLink *link)
{
    if (obt_link_type(link) == OBT_LINK_TYPE_APPLICATION) {
        const GQuark *cats;
        gulong i, n;

        cats = obt_link_app_categories(link, &n);
        for (i = 0; i < n; ++i)
            category_remove(lb, cats[i], link);
    }
}

static void category_free(ObtLinkBaseCategory *lc)
{
    g_slist_free(lc->links);
    g_slice_free(ObtLinkBaseCategory, lc);
}

/*! Called when a change happens in the filesystem. */
static void update(ObtWatch *w, const gchar *base_path,
                   const gchar *sub_path,
                   ObtWatchNotifyType type,
                   gpointer data)
{
    ObtLinkBase *self = data;
    ObtLinkBaseEntry *add = NULL;
    ObtLinkBaseEntry *remove = NULL;
    ObtLinkBaseEntry *show, *hide;
    ObtLink *link;
    gchar *id, *full_path;
    GList *list, *it;
    GList *remove_it = NULL;
    gint *priority;

    if (!g_str_has_suffix(sub_path, ".desktop"))
        return; /* ignore non-.desktop files */

    id = obt_link_id_from_ddfile(sub_path);
    list = g_hash_table_lookup(self->base, id);
    full_path = g_build_filename(base_path, sub_path, NULL);

    switch (type) {
    case OBT_WATCH_SELF_REMOVED:
        break;
    case OBT_WATCH_ADDED:
        priority = g_hash_table_lookup(self->path_to_priority, base_path);

        /* make sure an entry doesn't already exist from the same
           @base_path */
        it = find_base_entry_priority(list, *priority);
        if (it) {
            ObtLinkBaseEntry *e = it->data;
            if (e->priority == *priority) {
                break;
            }
        }
    case OBT_WATCH_MODIFIED:
        link = obt_link_from_ddfile(full_path, self->paths,
                                    self->language, self->country,
                                    self->modifier);
        if (link && !obt_link_display(link, self->environments)) {
            obt_link_unref(link);
            link = NULL;
        }
        if (link) {
            add = g_slice_new(ObtLinkBaseEntry);
            add->priority = *priority;
            add->link = link;
        }

        if (type != OBT_WATCH_MODIFIED)
            break;
    case OBT_WATCH_REMOVED:
        /* this may be NULL if the link was skipped during the add because
           it did not want to be displayed */
        remove_it = find_base_entry_path(list, full_path);
        remove = remove_it ? remove_it->data : NULL;
        break;
    }

    /* figure out which entry should be shown (which will have highest
       precedence) */
    show = hide = NULL;
    if (add) {
        ObtLinkBaseEntry *first = list ? list->data : NULL;

        /* a greater priority means a lower precedence, so
           the new one will replace the current front of the list */
        if (!first || first->priority >= add->priority) {
            show = add;
            hide = first;
        }
    }
    else if (remove) {
        ObtLinkBaseEntry *first = list ? list->data : NULL;

        if (first == remove) {
            hide = first;
            show = list->next ? list->next->data : NULL;
        }
    }

    if (hide)
        category_remove_app(self, hide->link);
    if (show)
        category_add_app(self, show->link);

    if ((remove || add) && self->update_func) {
        ObtLink *const a = show ? show->link : NULL;
        ObtLink *const r = hide ? hide->link : NULL;
        self->update_func(
            self, r, a, self->update_data);
    }

    /* do the actual removal/addition to the base list for the @id */
    if (remove_it) {
        base_entry_free(remove_it->data);
        list = g_list_delete_link(list, remove_it);
    }
    if (add) {
        it = find_base_entry_priority(list, *priority);
        list = g_list_insert_before(list, it, add);
    }

    if (remove || add) {
        /* update the list in the hash table */
        if (list) {
            /* this will free 'id' */
            g_hash_table_insert(self->base, id, list);
            id = NULL;
        }
        else {
            /* the value is already freed by deleting it above so we don't
               need to free it here. id will still need to be freed tho. */
            g_hash_table_steal(self->base, id);
        }
    }
    g_free(full_path);
    g_free(id);
}

ObtLinkBase* obt_linkbase_new(ObtPaths *paths, const gchar *locale,
                              guint environments)
{
    ObtLinkBase *self;
    GSList *it;
    gint priority;
    gint i;
    
    self = g_slice_new0(ObtLinkBase);
    self->environments = environments;
    self->watch = obt_watch_new();
    self->base = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    self->path_to_priority = g_hash_table_new_full(
        g_str_hash, g_str_equal, g_free, g_free);
    self->categories = g_hash_table_new_full(
        g_int_hash, g_int_equal, NULL, (GDestroyNotify)category_free);
    self->paths = paths;
    obt_paths_ref(paths);

    /* parse the locale string to determine the language, country, and
       modifier settings */
    for (i = 0; ; ++i)
        if (!locale[i] || locale[i] == '_' || locale[i] == '.' ||
            locale[i] == '@')
        {
            self->language = g_strndup(locale, i);
            break;
        }
        else if (((guchar)locale[i] < 'A' || (guchar)locale[i] > 'Z') &&
                 ((guchar)locale[i] < 'a' || (guchar)locale[i] > 'z'))
            break;
    if (self->language && locale[i] == '_') {
        locale += i+1;
        for (i = 0; ; ++i)
            if (!locale[i] || locale[i] == '.' || locale[i] == '@') {
                self->country = g_strndup(locale, i);
                break;
            }
            else if (((guchar)locale[i] < 'A' || (guchar)locale[i] > 'Z') &&
                     ((guchar)locale[i] < 'a' || (guchar)locale[i] > 'z'))
                break;
    }
    if (self->country && locale[i] == '.')
        for (; ; ++i)
            if (!locale[i] || locale[i] == '@')
                break;
            else if (((guchar)locale[i] < 'A' || (guchar)locale[i] > 'Z') &&
                     ((guchar)locale[i] < 'a' || (guchar)locale[i] > 'z'))
                break;
    if (self->country && locale[i] == '@') {
        locale += i+1;
        for (i = 0; ; ++i)
            if (!locale[i]) {
                self->modifier = g_strndup(locale, i);
                break;
            }
            else if (((guchar)locale[i] < 'A' || (guchar)locale[i] > 'Z') &&
                     ((guchar)locale[i] < 'a' || (guchar)locale[i] > 'z'))
                break;
    }

    /* run through each directory, foo, in the XDG_DATA_DIRS, and add
       foo/applications to the list of directories to watch here, with
       increasing priority (decreasing importance). */
    priority = 0;
    for (it = obt_paths_data_dirs(paths); it; it = g_slist_next(it)) {
        if (!g_hash_table_lookup(self->path_to_priority, it->data)) {
            gchar *base_path;
            gint *pri;

            base_path = g_build_filename(it->data, "applications", NULL);

            /* add to the hash table before adding the watch. the new watch
               will be calling our update handler, immediately with any
               files present in the directory */
            pri = g_new(gint, 1);
            *pri = priority;
            g_hash_table_insert(self->path_to_priority,
                                g_strdup(base_path), pri);

            obt_watch_add(self->watch, base_path, FALSE, update, self);

            ++priority;
        }
    }
    return self;
}

void obt_linkbase_ref(ObtLinkBase *self)
{
    ++self->ref;
}

void obt_linkbase_unref(ObtLinkBase *self)
{
    if (--self->ref < 1) {
        /* free all the values in the hash table
           we can't do this with a value_destroy_function in the hash table,
           because when we replace values, we are doing so with the same list
           (modified), and that would cause it to free the list we are putting
           back in.
         */
        g_hash_table_foreach(self->base, base_entry_list_free, NULL);

        obt_watch_unref(self->watch);
        g_hash_table_unref(self->categories);
        g_hash_table_unref(self->path_to_priority);
        g_hash_table_unref(self->base);
        obt_paths_unref(self->paths);
        g_slice_free(ObtLinkBase, self);
    }
}

void obt_linkbase_refresh(ObtLinkBase *lb)
{
    obt_watch_refresh(lb->watch);
}

void obt_linkbase_set_update_func(ObtLinkBase *lb, ObtLinkBaseUpdateFunc func,
                                  gpointer data)
{
    lb->update_func = func;
    lb->update_data = data;
}

GSList *obt_linkbase_category(ObtLinkBase *lb, GQuark category)
{
    ObtLinkBaseCategory *cat;

    cat = g_hash_table_lookup(lb->categories, &category);
    if (!cat) return NULL;
    else      return cat->links;
}
