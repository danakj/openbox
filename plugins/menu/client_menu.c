#include <glib.h>

#include "kernel/menu.h"
#include "kernel/screen.h"

static char *PLUGIN_NAME = "client_menu";
static Menu *send_to_menu;

typedef struct {

} Client_Menu_Data;

#define CLIENT_MENU(m) ((Menu *)m)
#define CLIENT_MENU_DATA(m) ((Client_Menu_Data *)((Menu *)m)->plugin_data)


void client_menu_clean_up(Menu *m) {
}

void client_send_to_update(Menu *self)
{
    guint i;
    g_message("yo!");
    
    for (i = 0; i < screen_num_desktops; ++i) {
        MenuEntry *e;
        Action *a = action_from_string("sendtodesktop");
        a->data.sendto.desk = i;
        a->data.sendto.follow = FALSE;
        e = menu_entry_new(screen_desktop_names[i], a);
        menu_add_entry(self, e);
    }

    menu_render_full(self);
}

void plugin_setup_config() { }

void plugin_shutdown() { }

void plugin_destroy (Menu *m)
{
}

void *plugin_create() /* TODO: need config */
{
    Menu *m = menu_new(NULL, "client-menu", NULL);
    menu_add_entry(m, menu_entry_new_submenu("Send To Workspace",
                                             send_to_menu));
    send_to_menu->parent = m;

    menu_add_entry(m, menu_entry_new("Iconify",
                                     action_from_string("iconify")));
    menu_add_entry(m, menu_entry_new("Raise",
                                     action_from_string("raise")));
    menu_add_entry(m, menu_entry_new("Lower",
                                     action_from_string("lower")));
    menu_add_entry(m, menu_entry_new("Close",
                                     action_from_string("close")));
    menu_add_entry(m, menu_entry_new("Shade",
                                     action_from_string("toggleshade")));
    menu_add_entry(m, menu_entry_new("Omnipresent",
                                     action_from_string("toggleomnipresent")));

    /* send to desktop
       iconify
       raise
       lower
       close
       kill
       shade
       omnipresent
       decorations
    */
    return (void *)m;
}

void plugin_startup()
{
    Menu *t;
    /* create a Send To Workspace Menu */
    send_to_menu = menu_new_full("Send To Workspace", "send-to-workspace",
                          NULL, NULL, client_send_to_update);

    t = (Menu *)plugin_create("client_menu");
}

