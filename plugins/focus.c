#include "../kernel/dispatch.h"
#include "../kernel/screen.h"
#include "../kernel/client.h"
#include "../kernel/frame.h"
#include "../kernel/focus.h"
#include "../kernel/stacking.h"
#include "../kernel/openbox.h"

/* config options */
static gboolean follow_mouse = TRUE;
static gboolean warp_on_desk_switch = FALSE;
static gboolean focus_new = TRUE;

/*static int skip_enter = 0;*/

static gboolean focus_under_pointer()
{
    Window w;
    int i, x, y;
    guint u;
    GList *it;

    if (XQueryPointer(ob_display, ob_root, &w, &w, &x, &y, &i, &i, &u))
    {
        for (it = stacking_list; it != NULL; it = it->next) {
            Client *c = it->data;
            if (c->desktop == screen_desktop &&
                RECT_CONTAINS(c->frame->area, x, y))
                break;
        }
        if (it != NULL) {
            return client_normal(it->data) && client_focus(it->data);
        }
    }
    return FALSE;
}

static void chew_enter_events()
{
    XEvent e;

    /* XXX... not anymore
       skip the next enter event from the desktop switch so focus
       doesn't skip briefly to what was under the pointer */

    /* kill all enter events from prior to the desktop switch, we
       aren't interested in them if we have found our own target
       to focus.
       XXX this is rude to other plugins...can this be done
       better? count the events in the queue? */
    while (XCheckTypedEvent(ob_display, EnterNotify, &e));
/*
    {
        XPutBackEvent(ob_display, &e);
        g_message("skip");
        ++skip_enter;
    }
*/
}

static void focus_fallback(gboolean switching_desks)
{
    GList *it;

    for (it = focus_order[screen_desktop]; it != NULL; it = it->next)
        if (client_normal(it->data) && client_focus(it->data)) {
            if (switching_desks) {
                Client *c = it->data;

                chew_enter_events();

                if (warp_on_desk_switch) {
                    /* I have to do this warp twice! Otherwise windows dont get
                       Enter/Leave events when i warp on a desktop switch! */
                    XWarpPointer(ob_display, None, c->window, 0, 0, 0, 0,
                                 c->area.width / 2, c->area.height / 2);
                    XWarpPointer(ob_display, None, c->window, 0, 0, 0, 0,
                                 c->area.width / 2, c->area.height / 2);
                }
            }
            break;
        }
}

static void focus_desktop()
{
    GList *it;

    for (it = g_list_last(stacking_list); it != NULL; it = it->prev) {
        Client *client = it->data;
        if (client->type == Type_Desktop && client->frame->visible)
            if (client_focus(client))
                break;
    }
    chew_enter_events();
}

static void event(ObEvent *e, void *foo)
{
    switch (e->type) {
    case Event_Client_Mapped:
        if (focus_new && client_normal(e->data.c.client))
            client_focus(e->data.c.client);
        break;

    case Event_Client_Unmapped:
        if (ob_state == State_Exiting) break;

        if (client_focused(e->data.c.client))
            if (!follow_mouse || !focus_under_pointer())
                focus_fallback(FALSE);
        break;

    case Event_Client_Desktop:
        /* focus the next available target if moving from the current
           desktop. */
        if ((unsigned)e->data.c.num[1] == screen_desktop)
            if (!follow_mouse || !focus_under_pointer())
                focus_fallback(FALSE);

    case Event_Ob_Desktop:
        focus_fallback(TRUE);
        break;

    case Event_Ob_ShowDesktop:
        if (!e->data.o.num[0]) { /* hiding the desktop, showing the clients */
            if (!follow_mouse || !focus_under_pointer())
                focus_fallback(TRUE);
        } else /* hiding clients, showing the desktop */
            focus_desktop();
        break;

    case Event_X_EnterNotify:
/*        if (skip_enter) {
            if (e->data.x.client != NULL)
                g_message("skipped enter %lx", e->data.x.client->window);
            else
                g_message("skipped enter 'root'");
            --skip_enter;
        }
        else*/
        if (e->data.x.client != NULL && client_normal(e->data.x.client))
            client_focus(e->data.x.client);
        break;

    default:
        g_assert_not_reached();
    }
}

void plugin_startup()
{
    dispatch_register(Event_Client_Mapped | 
                      Event_Ob_Desktop | 
                      Event_Client_Unmapped |
                      Event_X_EnterNotify |
                      Event_Ob_ShowDesktop,
                      (EventHandler)event, NULL);
}

void plugin_shutdown()
{
    dispatch_register(0, (EventHandler)event, NULL);
}
