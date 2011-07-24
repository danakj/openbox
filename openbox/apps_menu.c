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
#include "obt/bsearch.h"
#include "obt/linkbase.h"
#include "obt/link.h"
#include "obt/paths.h"

#include <glib.h>

#define MENU_NAME "apps-menu"

typedef struct _ObAppsMenuCategory ObAppsMenuCategory;

struct _ObAppsMenuCategory {
    /*! The quark for the internal name of the category, eg. AudioVideo. These
      come from the XDG menu specification. */
    GQuark q;
    /*! The user-visible (translated) name for the category. */
    const gchar *friendly;
    /*! The string identifier of the menu */
    gchar *menu_name;
    /*! The submenu for this category.  Its label will be "apps-menu-<category>
      where <category> is the internal name of the category referred to by q.
    */
    ObMenu *menu;
    /*! The menu entry in the apps menu for this. */
    ObMenuEntry *entry;
    /*! */
    GPtrArray *links;
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
    guint i;
    gboolean sort;

    if (!dirty) return TRUE;  /* nothing has changed to be updated */

    sort = FALSE;
    for (i = 0; i < n_categories; ++i) {
        ObAppsMenuCategory *cat = &categories[i];
        GList *it, *entries;
        guint j;
        gboolean sort_submenu;

        sort_submenu = FALSE; /* sort the category's submenu? */

        /* add/remove categories from the main apps menu if they were/are
           empty. */
        if (cat->links->len == 0) {
            if (cat->entry) {
                menu_entry_remove(cat->entry);
                cat->entry = NULL;
            }
        }
        else if (!cat->entry) {
            cat->entry = menu_add_submenu(apps_menu, 0, cat->menu_name);
            sort = TRUE;
        }

        /* add/remove links from this category. */
        entries = g_list_copy(cat->menu->entries);
        it = entries; /* iterates through the entries list */
        j = 0; /* iterates through the valid ObtLinks list */
        while (j < cat->links->len) {
            ObtLink *m, *l;
            int r;

            /* get the next link from the menu */
            if (it)
                m = ((ObMenuEntry*)it->data)->data.normal.data;
            else
                m = NULL;
            /* get the next link from our list */
            l = g_ptr_array_index(cat->links, j);

            if (it && m == NULL) {
                GList *next = it->next;

                /* the entry was removed from the linkbase and nulled here, so
                   drop it from the menu */
                menu_entry_remove(it->data);
                r = 0;

                /* don't move forward in the category list, as we're removing
                   something from the menu here, but do continue moving
                   in the menu in a safe way. */
                it = next;
            }
            else {
                if (it)
                    r = obt_link_cmp_by_name(&m, &l);
                else
                    /* we reached the end of the menu, but there's more in
                       the list, so just add it */
                    r = 1;

                if (r > 0) {
                    /* the menu progressed faster than the list, the list has
                       something it doesn't, so add that */
                    ObMenuEntry *e;
                    e = menu_add_normal(
                        cat->menu, 0, obt_link_name(l), NULL, FALSE);
                    e->data.normal.data = l; /* save the link in the entry */
                    sort_submenu = TRUE;
                }
                else
                    /* if r < 0 then we didn't null something that was removed
                       from the list */
                    g_assert(r == 0);

                if (r == 0)
                    /* if we did not add anything ot the menu, then move
                       forward in the menu. otherwise, stay put as we inserted
                       something just before this item, and want to compare
                       it again. */
                    it = g_list_next(it);
                /* the category link was either found in the menu or added to
                   it, so move to the next */
                ++j;
            }
        }

        if (sort_submenu)
            menu_sort_entries(cat->menu);
    }

    if (sort) {
        menu_sort_entries(apps_menu);
    }

    dirty = FALSE;
    return TRUE; /* always show the menu */
}

static gboolean add_link_to_category(ObtLink *link, ObAppsMenuCategory *cat)
{
    guint i;
    gboolean add;

    /* check for link in our existing list */
    add = TRUE;
    for (i = 0; i < cat->links->len; ++i) {
        ObtLink *l = g_ptr_array_index(cat->links, i);
        if (l == link) {
            add = FALSE;
            break;
        }
    }

    if (add) { /* wasn't found in our list already */
        g_ptr_array_add(cat->links, link);
        obt_link_ref(link);

        dirty = TRUE; /* never set dirty to FALSE here.. */
    }
    return add;
}

static gboolean remove_link_from_category(ObtLink *link,
                                          ObAppsMenuCategory *cat)
{
    gboolean rm;

    rm = g_ptr_array_remove(cat->links, link);

    if (rm) {
        GList *it;

        dirty = TRUE; /* never set dirty to FALSE here.. */

        /* set the data inside the category's menu for this link to NULL
           since it stops existing now */
        if (cat->menu) {
            for (it = cat->menu->entries; it; it = g_list_next(it)) {
                ObMenuEntry *e = it->data;
                /* our submenus only contain normal entries */
                if (e->data.normal.data == link) {
                    /* this menu entry points to this link, but the
                       link is going away, so point it at NULL. */
                    e->data.normal.data = NULL;
                    break;
                }
            }
        }
    }
    return rm;
}

static void linkbase_update(ObtLinkBase *lb, ObtLink *removed,
                            ObtLink *added, gpointer data)
{
    const GQuark *cats;
    gulong n_cats, i;

    /* For each category in the link, search our list of categories and
       if we are showing that category in the menu, add the link to it (or
       remove the link from it). */

#define CAT_TO_INT(c) (c.q)
    if (removed) {
        cats = obt_link_app_categories(removed, &n_cats);
        for (i = 0; i < n_cats; ++i) {
            BSEARCH_SETUP();
            BSEARCH_CMP(ObAppsMenuCategory, categories, 0, n_categories,
                    cats[i], CAT_TO_INT);
            if (BSEARCH_FOUND())
                remove_link_from_category(removed, &categories[BSEARCH_AT()]);
        }
    }

    if (added) {
        cats = obt_link_app_categories(added, &n_cats);
        for (i = 0; i < n_cats; ++i) {
            BSEARCH_SETUP();
            BSEARCH_CMP(ObAppsMenuCategory, categories, 0, n_categories,
                        cats[i], CAT_TO_INT);
            if (BSEARCH_FOUND()) {
                add_link_to_category(added, &categories[BSEARCH_AT()]);
                g_ptr_array_sort(categories[BSEARCH_AT()].links,
                                 obt_link_cmp_by_name);
            }
        }
    }
#undef  CAT_TO_INT
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
    if (reconfig) {
        /* Force a re-read of the applications available in case we are not
           getting notifications about changes. */
        obt_linkbase_refresh(linkbase);
    }
    else {
        ObtPaths *paths;
        guint i;

        paths = obt_paths_new();
        /* XXX allow more environments, like GNOME or KDE, to be included */
        linkbase = obt_linkbase_new(paths, ob_locale_msg,
                                    OBT_LINK_ENV_OPENBOX);
        obt_paths_unref(paths);
        obt_linkbase_set_update_func(linkbase, linkbase_update, NULL);

        dirty = FALSE;

        /* From http://standards.freedesktop.org/menu-spec/latest/apa.html */
        {
            struct {
                const gchar *name;
                const gchar *friendly;
            } const cats[] = {
                { "AudioVideo", "Sound & Video" },
                { "Development", "Programming" },
                { "Education", "Education" },
                { "Game", "Games" },
                { "Graphics", "Graphics" },
                { "Network", "Internet" },
                { "Office", "Office" },
                { "Settings", "Settings" },
                { "System", "System" },
                { "Utility", "Accessories" },
                { NULL, NULL }
            };
            guint i;

            for (i = 0; cats[i].name; ++i); /* count them */
            n_categories = i;

            categories = g_new0(ObAppsMenuCategory, n_categories);
            sorted_categories = g_new(ObAppsMenuCategory*, n_categories);

            for (i = 0; cats[i].name; ++i) {
                categories[i].q = g_quark_from_static_string(cats[i].name);
                categories[i].friendly = _(cats[i].friendly);
                categories[i].menu_name = g_strdup_printf(
                    "%s-%s", MENU_NAME, cats[i].name);
                categories[i].menu = menu_new(categories[i].menu_name,
                                              categories[i].friendly, FALSE,
                                              &categories[i]);
                categories[i].links = g_ptr_array_new_with_free_func(
                    (GDestroyNotify)obt_link_unref);
            }
        }
        /* Sort the categories by their quark values so we can binary search */
        qsort(categories, n_categories, sizeof(ObAppsMenuCategory), cat_cmp);

        for (i = 0; i < n_categories; ++i)
            sorted_categories[i] = &categories[i];
        qsort(sorted_categories, n_categories, sizeof(void*),
              cat_friendly_cmp);

        /* add all the existing links */
        for (i = 0; i < n_categories; ++i) {
            GSList *links, *it;

            links = obt_linkbase_category(linkbase, categories[i].q);
            for (it = links; it; it = g_slist_next(it))
                add_link_to_category(it->data, &categories[i]);

            g_ptr_array_sort(categories[i].links, obt_link_cmp_by_name);
        }
    }

    apps_menu = menu_new(MENU_NAME, _("Applications"), FALSE, NULL);
    menu_set_update_func(apps_menu, self_update);
    menu_set_destroy_func(apps_menu, self_destroy);
}

void apps_menu_shutdown(gboolean reconfig)
{
    if (!reconfig) {
        guint i;

        obt_linkbase_unref(linkbase);
        linkbase = NULL;

        for (i = 0; i < n_categories; ++i) {

            menu_free(categories[i].menu);
            g_free(categories[i].menu_name);
            g_ptr_array_unref(categories[i].links);
        }
        g_free(categories);
        categories = NULL;
        g_free(sorted_categories);
        sorted_categories = NULL;
    }

    /* freed by the hash table */
}
