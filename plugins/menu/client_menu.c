#include "kernel/debug.h"
#include "kernel/menu.h"
#include "kernel/screen.h"
#include "kernel/client.h"
#include "kernel/openbox.h"
#include "kernel/frame.h"

#include "render/theme.h"

#include <glib.h>

static char *PLUGIN_NAME = "client_menu";

static ObMenu *send_to_menu;
static ObMenu *layer_menu;

typedef struct {
    gint foo;
} Client_Menu_Data;

#define CLIENT_MENU(m) ((ObMenu *)m)
#define CLIENT_MENU_DATA(m) ((Client_Menu_Data *)((ObMenu *)m)->plugin_data)

void client_menu_clean_up(ObMenu *m) {
}

void client_send_to_update(ObMenu *self)
{
    guint i = 0;
    GList *it = self->entries;
    
    /* check if we have to update. lame */
    while (it != NULL) {
        if (i >= screen_num_desktops)
            break;
        if (strcmp(screen_desktop_names[i],
                   ((ObMenuEntry *)it->data)->label) != 0)
            break;
        ++i;
        it = it->next;
    }

    if (it != NULL || i != screen_num_desktops) {
        menu_clear(self);
        ob_debug("update\n");
        for (i = 0; i < screen_num_desktops; ++i) {
            ObMenuEntry *e;
            Action *a = action_from_string("sendtodesktop");
            a->data.sendto.desk = i;
            a->data.sendto.follow = FALSE;
            e = menu_entry_new(screen_desktop_names[i], a);
            menu_add_entry(self, e);
        }
        
        self->update(self);
    }
}

void client_menu_show(ObMenu *self, int x, int y, ObClient *client)
{
    guint i;
    gint newy, newx;
    Rect *a = NULL;

    g_assert(!self->invalid);
    g_assert(client);
    
    for (i = 0; i < screen_num_monitors; ++i) {
        a = screen_physical_area_monitor(i);
        if (RECT_CONTAINS(*a, x, y))
            break;
    }
    g_assert(a != NULL);
    self->xin_area = i;

    newx = MAX(x, client->area.x);
    newy = MAX(y, client->area.y);
    POINT_SET(self->location,
	      MIN(newx, client->area.x + client->area.width - self->size.width),
	      MIN(newy, client->area.y + client->area.height -
                  self->size.height));
    
    XMoveWindow(ob_display, self->frame, self->location.x, self->location.y);

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

void plugin_destroy (ObMenu *m)
{
}

void *plugin_create() /* TODO: need config */
{
    ObMenu *m = menu_new_full(NULL, "client-menu", NULL,
                            client_menu_show, NULL, NULL, NULL, NULL);
    m->plugin = PLUGIN_NAME;
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
    menu_add_entry(m, menu_entry_new("Decorations",
                                     action_from_string("toggledecorations")));
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
    ObMenu *t;
    /* create a Send To Workspace ObMenu */
    send_to_menu = menu_new_full(NULL, "send-to-workspace",
                          NULL, NULL, client_send_to_update, NULL, NULL, NULL);
    
    layer_menu = menu_new(NULL, "layer", NULL);
    menu_add_entry(layer_menu, menu_entry_new("Top Layer",
                                     action_from_string("sendtotoplayer")));
    menu_add_entry(layer_menu, menu_entry_new("Normal Layer",
                                     action_from_string("sendtonormallayer")));
    menu_add_entry(layer_menu, menu_entry_new("Bottom Layer",
                                     action_from_string("sendtobottomlayer")));
                          
    t = (ObMenu *)plugin_create("client_menu");
}

