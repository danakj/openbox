#include <glib.h>

#include "kernel/menu.h"
#include "kernel/screen.h"
#include "kernel/client.h"
#include "kernel/openbox.h"

#include "kernel/frame.h"

static char *PLUGIN_NAME = "client_menu";

static Menu *send_to_menu;
static Menu *layer_menu;

typedef struct {

} Client_Menu_Data;

#define CLIENT_MENU(m) ((Menu *)m)
#define CLIENT_MENU_DATA(m) ((Client_Menu_Data *)((Menu *)m)->plugin_data)


void client_menu_clean_up(Menu *m) {
}

void client_send_to_update(Menu *self)
{
    guint i;
    
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

void client_menu_show(Menu *self, int x, int y, Client *client)
{
    int newy;
    g_assert(!self->invalid);
    g_assert(client);
    
    newy = client->frame->area.y + client->frame->a_focused_title->area.height;
    
    XMoveWindow(ob_display, self->frame, 
		MIN(x, screen_physical_size.width - self->size.width), 
		MIN(newy, screen_physical_size.height - self->size.height));
    POINT_SET(self->location, 
	      MIN(x, screen_physical_size.width - self->size.width), 
	      MIN(newy, screen_physical_size.height - self->size.height));

    if (!self->shown) {
	XMapWindow(ob_display, self->frame);
        stacking_raise(MENU_AS_WINDOW(self));
	self->shown = TRUE;
    } else if (self->shown && self->open_submenu) {
	menu_hide(self->open_submenu);
    }
}

void plugin_setup_config() { }

void plugin_shutdown() { }

void plugin_destroy (Menu *m)
{
}

void *plugin_create() /* TODO: need config */
{
    Menu *m = menu_new_full(NULL, "client-menu", NULL,
                            client_menu_show, NULL);
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
    menu_add_entry(m, menu_entry_new_submenu("Layers",
                                             layer_menu));
    layer_menu->parent = m;

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
    send_to_menu = menu_new_full(NULL, "send-to-workspace",
                          NULL, NULL, client_send_to_update);
    
    layer_menu = menu_new(NULL, "layer", NULL);
    menu_add_entry(layer_menu, menu_entry_new("Top Layer",
                                     action_from_string("sendtotoplayer")));
    menu_add_entry(layer_menu, menu_entry_new("Normal Layer",
                                     action_from_string("sendtonormallayer")));
    menu_add_entry(layer_menu, menu_entry_new("Bottom Layer",
                                     action_from_string("sendtobottomlayer")));
                          
    t = (Menu *)plugin_create("client_menu");
}

