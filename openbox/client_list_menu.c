#include "openbox.h"
#include "menu.h"
#include "menuframe.h"
#include "action.h"
#include "screen.h"
#include "client.h"
#include "focus.h"
#include "gettext.h"

#include <glib.h>

#define MENU_NAME "client-list-menu"

static GSList *desktop_menus;

typedef struct {
    guint desktop;
} DesktopData;

static void desk_menu_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    DesktopData *d = data;
    GList *it;
    gint i;
    gboolean icons = FALSE;

    menu_clear_entries(menu);

    for (it = focus_order[d->desktop], i = 0; it; it = g_list_next(it), ++i) {
        ObClient *c = it->data;
        if (client_normal(c)) {
            GSList *acts;
            ObAction* act;
            ObMenuEntry *e;
            ObClientIcon *icon;

            if (!icons && c->iconic) {
                icons = TRUE;
                menu_add_separator(menu, -1);
            }

            act = action_from_string("activate");
            act->data.activate.any.c = c;
            acts = g_slist_prepend(NULL, act);
            e = menu_add_normal(menu, i,
                                (c->iconic ? c->icon_title : c->title), acts);

            if ((icon = client_icon(c, 32, 32))) {
                e->data.normal.icon_width = icon->width;
                e->data.normal.icon_height = icon->height;
                e->data.normal.icon_data = icon->data;
            }
        }
    }
    
}

/* executes it without changing the client in the actions, since we set that
   when we make the actions! */
static void desk_menu_execute(ObMenuEntry *self, gpointer data)
{
    GSList *it;

    for (it = self->data.normal.actions; it; it = g_slist_next(it))
    {
        ObAction *act = it->data;
        act->func(&act->data);
    }
}

static void desk_menu_destroy(ObMenu *menu, gpointer data)
{
    DesktopData *d = data;

    g_free(d);
}

static void self_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    guint i;
    GSList *it, *next;
    
    it = desktop_menus;
    for (i = 0; i < screen_num_desktops; ++i) {
        if (!it) {
            ObMenu *submenu;
            gchar *name = g_strdup_printf("%s-%u", MENU_NAME, i);
            DesktopData *data = g_new(DesktopData, 1);

            data->desktop = i;
            submenu = menu_new(name, screen_desktop_names[i], data);
            menu_set_update_func(submenu, desk_menu_update);
            menu_set_execute_func(submenu, desk_menu_execute);
            menu_set_destroy_func(submenu, desk_menu_destroy);

            menu_add_submenu(menu, i, name);

            g_free(name);

            desktop_menus = g_slist_append(desktop_menus, submenu);
        } else
            it = g_slist_next(it);
    }
    for (; it; it = next, ++i) {
        next = g_slist_next(it);
        menu_free(it->data);
        desktop_menus = g_slist_delete_link(desktop_menus, it);
        menu_entry_remove(menu_find_entry_id(menu, i));
    }
}

void client_list_menu_startup()
{
    ObMenu *menu;

    menu = menu_new(MENU_NAME, _("Desktops"), NULL);
    menu_set_update_func(menu, self_update);
}
