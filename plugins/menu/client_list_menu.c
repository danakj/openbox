#include "kernel/openbox.h"
#include "kernel/menu.h"
#include "kernel/menuframe.h"
#include "kernel/action.h"
#include "kernel/screen.h"
#include "kernel/client.h"
#include "kernel/focus.h"
#include "gettext.h"

#include "render/theme.h"

#include <glib.h>

#define MENU_NAME "client-list-menu"

typedef struct {
    /* how many desktop menus we've made */
    guint desktops;
} MenuData;

typedef struct {
    guint desktop;
} DesktopData;

void plugin_setup_config() { }

static void desk_menu_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    DesktopData *d = data;
    GList *it;
    gint i;

    menu_clear_entries(menu->name);

    for (it = focus_order[d->desktop], i = 0; it; it = g_list_next(it), ++i) {
        ObClient *c = it->data;
        if (client_normal(c)) {
            GSList *acts;
            ObAction* act;
            ObMenuEntry *e;
            ObClientIcon *icon;

            act = action_from_string("activate");
            act->data.activate.c = c;
            acts = g_slist_prepend(NULL, act);
            e = menu_add_normal(menu->name, i,
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
    guint i;
    MenuData *d = data;
    
    menu_clear_entries(MENU_NAME);

    for (i = 0; i < screen_num_desktops; ++i) {
        gchar *name = g_strdup_printf("%s-%u", MENU_NAME, i);
        DesktopData *data = g_new(DesktopData, 1);

        data->desktop = i;
        menu_new(name, screen_desktop_names[i], data);
        menu_set_update_func(name, desk_menu_update);
        menu_set_execute_func(name, desk_menu_execute);
        menu_set_destroy_func(name, desk_menu_destroy);

        menu_add_submenu(MENU_NAME, 0, name);

        g_free(name);
    }

    d->desktops = MAX(d->desktops, screen_num_desktops);
}

static void self_destroy(ObMenu *menu, gpointer data)
{
    MenuData *d = data;
    guint i;

    for (i = 0; i < d->desktops; ++i) {
        gchar *name = g_strdup_printf("%s-%u", MENU_NAME, i);
        menu_free(name);
        g_free(name);
    }
    g_free(d);
}

void plugin_startup()
{
    MenuData *data;

    data = g_new(MenuData, 1);
    data->desktops = 0;
    menu_new(MENU_NAME, _("Desktops"), data);
    menu_set_update_func(MENU_NAME, self_update);
    menu_set_destroy_func(MENU_NAME, self_destroy);
}

void plugin_shutdown()
{
    menu_free(MENU_NAME);
}
