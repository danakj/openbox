#include "dispatch.h"
#include "extensions.h"

#include <glib.h>

static GSList **funcs;

void dispatch_startup()
{
    guint i;
    EventType j;

    i = 0;
    j = EVENT_RANGE;
    while (j > 1) {
        j >>= 1;
        ++i;
    }
    funcs = g_new(GSList*, i);

    for (i = 0, j = 1; j < EVENT_RANGE; ++i, j <<= 1)
        funcs[i] = NULL;
}

void dispatch_shutdown()
{
    guint i;
    EventType j;

    for (i = 0, j = 1; j < EVENT_RANGE; ++i, j <<= 1) {
        g_slist_free(funcs[i]);
        funcs[i] = NULL;
    }

    g_free(funcs);
}

void dispatch_register(EventHandler h, EventMask mask)
{
    guint i;
    EventType j;

    while (mask) {
        for (i = 0, j = 1; j < EVENT_RANGE; ++i, j <<= 1)
            if (mask & j) {
                funcs[i] = g_slist_append(funcs[i], h);
                mask ^= j; /* remove from the mask */
            }
        g_assert(j >= EVENT_RANGE); /* an invalid event is in the mask */
    }
}

void dispatch_x(XEvent *xe, Client *c)
{
    EventType e;
    guint i;
    GSList *it;
    ObEvent obe;

    switch (xe->type) {
    case EnterNotify:
        e = Event_X_EnterNotify;
        break;
    case LeaveNotify:
        e = Event_X_LeaveNotify;
        break;
    case KeyPress:
        e = Event_X_KeyPress;
        break;
    case KeyRelease:
        e = Event_X_KeyRelease;
        break;
    case ButtonPress:
        e = Event_X_ButtonPress;
        break;
    case ButtonRelease:
        e = Event_X_ButtonRelease;
        break;
    case MotionNotify:
        e = Event_X_MotionNotify;
        break;
    default:
	/* XKB events */
	if (xe->type == extensions_xkb_event_basep) {
	    switch (((XkbAnyEvent*)&e)->xkb_type) {
	    case XkbBellNotify:
		e = Event_X_Bell;
		break;
	    }
	}
        return;
    }

    obe.type = e;
    obe.data.x.e = xe;
    obe.data.x.client = c;

    i = 0;
    while (e > 1) {
        e >>= 1;
        ++i;
    }

    for (it = funcs[i]; it != NULL; it = it->next)
        ((EventHandler)it->data)(&obe);
}

void dispatch_client(EventType e, Client *c)
{
    guint i;
    GSList *it;
    ObEvent obe;

    g_assert(c != NULL);

    obe.type = e;
    obe.data.client = c;

    i = 0;
    while (e > 1) {
        e >>= 1;
        ++i;
    }

    for (it = funcs[i]; it != NULL; it = it->next)
        ((EventHandler)it->data)(&obe);
}

void dispatch_ob(EventType e)
{
    guint i;
    GSList *it;
    ObEvent obe;

    obe.type = e;

    i = 0;
    while (e > 1) {
        e >>= 1;
        ++i;
    }

    for (it = funcs[i]; it != NULL; it = it->next)
        ((EventHandler)it->data)(&obe);
}

void dispatch_signal(int signal)
{
    guint i;
    EventType e = Event_Signal;
    GSList *it;
    ObEvent obe;

    obe.type = e;
    obe.data.signal = signal;

    i = 0;
    while (e > 1) {
        e >>= 1;
        ++i;
    }

    for (it = funcs[i]; it != NULL; it = it->next)
        ((EventHandler)it->data)(&obe);
}
