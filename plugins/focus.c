#include "../kernel/dispatch.h"
#include "../kernel/screen.h"
#include "../kernel/client.h"
#include "../kernel/focus.h"
#include "../kernel/stacking.h"
#include "../kernel/openbox.h"

static int skip_enter = 0;

static void focus_fallback(guint desk, gboolean warp)
{
    GList *it;

    for (it = focus_order[desk]; it != NULL; it = it->next)
        if (client_focus(it->data)) {
            if (warp) { /* XXX make this configurable */
                XEvent e;
                Client *c = it->data;

                /* skip the next enter event from the desktop switch so focus
                   doesn't skip briefly to what was under the pointer */
                if (XCheckTypedEvent(ob_display, EnterNotify, &e)) {
                    XPutBackEvent(ob_display, &e);
                    /* XXX WERE NOT SKIPPING THEM ALL@&*)! */
                    g_message("Skip");
                    ++skip_enter;
                }

                /* I have to do this warp twice! Otherwise windows dont get
                   Enter/Leave events when i warp on a desktop switch! */
                XWarpPointer(ob_display, None, c->window, 0, 0, 0, 0,
                             c->area.width / 2, c->area.height / 2);
                XWarpPointer(ob_display, None, c->window, 0, 0, 0, 0,
                             c->area.width / 2, c->area.height / 2);
            }
            break;
        }
}

static void events(ObEvent *e, void *foo)
{
    switch (e->type) {
    case Event_Client_Mapped:
        /* focus new normal windows */
        if (client_normal(e->data.c.client))
            client_focus(e->data.c.client);
        break;

    case Event_Ob_Desktop:
        /* focus the next available target */
        focus_fallback(e->data.o.num[0], TRUE);
        break;

    case Event_Client_Unfocus:
        /* dont do this shit with sloppy focus... */
        /*
        /\* nothing is left with focus! *\/
        if (focus_client == NULL) 
            /\* focus the next available target *\/
            focus_fallback(screen_desktop, FALSE);
        */
        break;

    case Event_X_EnterNotify:
        if (skip_enter)
            --skip_enter;
        else if (e->data.x.client && client_normal(e->data.x.client))
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
                      Event_Client_Unfocus |
                      Event_X_EnterNotify,
                      (EventHandler)events, NULL);
}

void plugin_shutdown()
{
    dispatch_register(0, (EventHandler)events, NULL);
}
