#include "kernel/openbox.h"
#include "kernel/menu.h"
#include "kernel/action.h"
#include "kernel/screen.h"
#include "kernel/client.h"
#include "kernel/focus.h"

#include "render/theme.h"

#include <glib.h>

static char *PLUGIN_NAME = "client_list_menu";

typedef struct {
    GSList *submenus;
} Client_List_Menu_Data;

typedef struct {
    guint desktop;
} Client_List_Desktop_Menu_Data;

#define CLIENT_LIST_MENU(m) ((ObMenu *)m)
#define CLIENT_LIST_MENU_DATA(m) ((Client_List_Menu_Data *)((ObMenu *)m)->plugin_data)

#define CLIENT_LIST_DESKTOP_MENU(m) ((ObMenu *)m)
#define CLIENT_LIST_DESKTOP_MENU_DATA(m) ((Client_List_Desktop_Menu_Data *)((ObMenu *)m)->plugin_data)

static void self_update(ObMenu *self);
static void self_destroy(ObMenu *self);

void plugin_setup_config() { }
void plugin_shutdown() { }
void plugin_destroy (ObMenu *m) { }

void *plugin_create()
{
    ObMenu *menu = menu_new_full("Desktops", "client-list-menu", NULL,
                                 NULL, self_update, NULL,
                                 NULL, NULL, self_destroy);

    menu->plugin = PLUGIN_NAME;
    menu->plugin_data = g_new(Client_List_Menu_Data, 1);
    CLIENT_LIST_MENU_DATA(menu)->submenus = NULL;

    return (void *)menu;
}

void plugin_startup()
{
    plugin_create("client_list_menu");
}


static void desk_update(ObMenu *self)
{
    GList *it;
    guint desk;

    menu_clear(self);

    desk = CLIENT_LIST_DESKTOP_MENU_DATA(self)->desktop;

    for (it = focus_order[desk]; it; it = g_list_next(it)) {
        ObClient *c = (ObClient *)it->data;
        if (client_normal(c)) {
            ObAction* a = action_from_string("activate");
            a->data.activate.c = c;
            menu_add_entry(self, menu_entry_new((c->iconic ?
                                                 c->icon_title :
                                                 c->title), a));
        }
    }

    menu_render(self);
}

static void desk_selected(ObMenuEntry *entry,
                          unsigned int button, unsigned int x, unsigned int y)
{
    entry->action->data.activate.here = (button == 2);
    entry->parent->client = entry->action->data.activate.c;
    menu_entry_fire(entry, button, x, y);
}

static void desk_destroy(ObMenu *self)
{
    g_free(self->plugin_data);
}

static void self_update(ObMenu *self)
{
    guint i, n;
    ObMenu *deskmenu;
    gchar *s;
    GList *eit, *enext;
    GSList *sit, *snext;

    n = g_slist_length(CLIENT_LIST_MENU_DATA(self)->submenus);

    for (i = 0; i < screen_num_desktops; ++i) {
        if (i >= n) {
            s = g_strdup_printf("client-list-menu-desktop-%d", i);
            deskmenu = menu_new_full(screen_desktop_names[i], s, self,
                                     NULL,
                                     desk_update, desk_selected, NULL, NULL,
                                     desk_destroy);
            g_free(s);

            deskmenu->plugin = PLUGIN_NAME;
            deskmenu->plugin_data = g_new(Client_List_Desktop_Menu_Data, 1);
            CLIENT_LIST_DESKTOP_MENU_DATA(deskmenu)->desktop = i;

            CLIENT_LIST_MENU_DATA(self)->submenus =
                g_slist_append(CLIENT_LIST_MENU_DATA(self)->submenus,
                               deskmenu);
        }

        menu_add_entry(self, menu_entry_new_submenu(screen_desktop_names[i],
                                                    deskmenu));
    }

    for (eit = g_list_nth(self->entries, i); eit; eit = enext) {
        enext = g_list_next(eit);
	menu_entry_free(eit->data);
        self->entries = g_list_delete_link(self->entries, eit);
    }

    for (sit = g_slist_nth(CLIENT_LIST_MENU_DATA(self)->submenus, i);
         sit; sit = snext) {
        snext = g_slist_next(sit);
        menu_free(sit->data);
        CLIENT_LIST_MENU_DATA(self)->submenus = 
            g_slist_delete_link(CLIENT_LIST_MENU_DATA(self)->submenus, sit);
    }

    menu_render(self);
}

static void self_destroy(ObMenu *self)
{
    g_free(self->plugin_data);
}
