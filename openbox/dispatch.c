#include "dispatch.h"
#include "extensions.h"

#include <glib.h>

typedef struct {
    EventHandler h;
    void *data;
} Func;

/* an array of GSList*s of Func*s */
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
    GSList *it;

    for (i = 0, j = 1; j < EVENT_RANGE; ++i, j <<= 1) {
        for (it = funcs[i]; it != NULL; it = it->next)
            g_free(it->data);
        g_slist_free(funcs[i]);
        funcs[i] = NULL;
    }

    g_free(funcs);
}

void dispatch_register(EventMask mask, EventHandler h, void *data)
{
    guint i;
    EventType j;
    GSList *it, *next;
    EventMask m;
    Func *f;

    /* add to masks it needs to be registered for */
    m = mask;
    while (m) {
        for (i = 0, j = 1; j < EVENT_RANGE; ++i, j <<= 1)
            if (m & j) {
                for (it = funcs[i]; it != NULL; it = it->next) {
                    f = it->data;
                    if (f->h == h && f->data == data)
                        break;
                }
                if (it == NULL) { /* wasn't already regged */
                    f = g_new(Func, 1);
                    f->h = h;
                    f->data = data;
                    funcs[i] = g_slist_append(funcs[i], f);
                }
                m ^= j; /* remove from the mask */
            }
        g_assert(j >= EVENT_RANGE); /* an invalid event is in the mask */
    }

    /* remove from masks its not registered for anymore */
    for (i = 0, j = 1; j < EVENT_RANGE; ++i, j <<= 1) {
        if (!(j & mask))
            for (it = funcs[i]; it != NULL; it = next) {
                next = it->next;
                f = it->data;
                if (f->h == h && f->data == data) {
                    g_free(f);
                    funcs[i] = g_slist_delete_link(funcs[i], it);
                }
            }
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

    for (it = funcs[i]; it != NULL; it = it->next) {
        Func *f = it->data;
        f->h(&obe, f->data);
    }
}

void dispatch_client(EventType e, Client *c, int num0, int num1)
{
    guint i;
    GSList *it;
    ObEvent obe;

    g_assert(c != NULL);

    obe.type = e;
    obe.data.c.client = c;
    obe.data.c.num[0] = num0;
    obe.data.c.num[1] = num1;

    i = 0;
    while (e > 1) {
        e >>= 1;
        ++i;
    }

    for (it = funcs[i]; it != NULL; it = it->next) {
        Func *f = it->data;
        f->h(&obe, f->data);
    }
}

void dispatch_ob(EventType e, int num0, int num1)
{
    guint i;
    GSList *it;
    ObEvent obe;

    obe.type = e;
    obe.data.o.num[0] = num0;
    obe.data.o.num[1] = num1;

    i = 0;
    while (e > 1) {
        e >>= 1;
        ++i;
    }

    for (it = funcs[i]; it != NULL; it = it->next) {
        Func *f = it->data;
        f->h(&obe, f->data);
    }
}

void dispatch_signal(int signal)
{
    guint i;
    EventType e = Event_Signal;
    GSList *it;
    ObEvent obe;

    obe.type = e;
    obe.data.s.signal = signal;

    i = 0;
    while (e > 1) {
        e >>= 1;
        ++i;
    }

    for (it = funcs[i]; it != NULL; it = it->next) {
        Func *f = it->data;
        f->h(&obe, f->data);
    }
}

void dispatch_move(Client *c, int *x, int *y)
{
    guint i;
    EventType e = Event_Client_Moving;
    GSList *it;
    ObEvent obe;

    obe.type = e;
    obe.data.c.client = c;
    obe.data.c.num[0] = *x;
    obe.data.c.num[1] = *y;

    i = 0;
    while (e > 1) {
        e >>= 1;
        ++i;
    }

    for (it = funcs[i]; it != NULL; it = it->next) {
        Func *f = it->data;
        f->h(&obe, f->data);
    }

    *x = obe.data.c.num[0];
    *y = obe.data.c.num[1];
}
