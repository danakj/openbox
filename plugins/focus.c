#include "../kernel/dispatch.h"
#include "../kernel/screen.h"
#include "../kernel/client.h"
#include "../kernel/stacking.h"

static GSList **focus_order = NULL;

static void events(ObEvent *e, void *foo)
{
    guint i;
    guint new, old;
    GSList *it;

    switch (e->type) {
    case Event_Client_Mapped:
        /* focus new normal windows */
        if (client_normal(e->data.c.client))
            client_focus(e->data.c.client);
        break;

    case Event_Ob_NumDesktops:
        new = e->data.o.num[0];
        old = e->data.o.num[1];
        /* free our lists for the desktops which have disappeared */
        for (i = new; i < old; ++i)
            g_slist_free(focus_order[i]);
        /* realloc the array */
        focus_order = g_renew(GSList*, focus_order, new);
        /* set the new lists to be empty */
        for (i = old; i < new; ++i)
            focus_order[i] = NULL;
        break;

    case Event_Client_Desktop:
        old = e->data.c.num[1];
        if (old != DESKTOP_ALL)
            focus_order[old] = g_slist_remove(focus_order[old],
                                              e->data.c.client);
        else
            for (i = 0; i < screen_num_desktops; ++i)
                focus_order[i] = g_slist_remove(focus_order[i],
                                                e->data.c.client);
        break;

    case Event_Ob_Desktop:
        for (it = focus_order[e->data.o.num[0]]; it != NULL; it = it->next)
            if (client_focus(it->data))
                break;
        break;

    case Event_Client_Focus:
        /* move to the top of the list */
        focus_order[e->data.c.num[1]] =
        g_slist_remove(focus_order[e->data.c.num[1]], e->data.c.client);
        focus_order[e->data.c.num[1]] =
        g_slist_prepend(focus_order[e->data.c.num[1]], e->data.c.client);
        break;

    default:
        g_assert_not_reached();
    }
}

void plugin_startup()
{
    guint i;

    dispatch_register(Event_Client_Mapped | Event_Ob_Desktop |
                      Event_Ob_NumDesktops | Event_Client_Focus |
                      Event_Client_Desktop, (EventHandler)events, NULL);

    focus_order = g_new(GSList*, screen_num_desktops);
    for (i = 0; i < screen_num_desktops; ++i)
        focus_order[i] = NULL;
}

void plugin_shutdown()
{
    guint i;

    dispatch_register(0, (EventHandler)events, NULL);

    for (i = 0; i < screen_num_desktops; ++i)
        g_slist_free(focus_order[i]);
    g_free(focus_order);
}
