#include "mbind.h"
#include "kbind.h"
#include "frame.h"
#include "openbox.h"
#include "eventdata.h"
#include "hooks.h"

#include <glib.h>
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

/* GData of GSList*'s of PointerBinding*'s. */
static GData *bound_contexts;

static gboolean grabbed;

struct mbind_foreach_grab_temp {
    Client *client;
    gboolean grab;
};

typedef struct {
    guint state;
    guint button;
    char *name;
} PointerBinding;

static gboolean translate(char *str, guint *state, guint *button)
{
    char **parsed;
    char *l;
    int i;
    gboolean ret = FALSE;

    parsed = g_strsplit(str, "-", -1);
    
    /* first, find the button (last token) */
    l = NULL;
    for (i = 0; parsed[i] != NULL; ++i)
	l = parsed[i];
    if (l == NULL)
	goto translation_fail;

    /* figure out the mod mask */
    *state = 0;
    for (i = 0; parsed[i] != l; ++i) {
	guint m = kbind_translate_modifier(parsed[i]);
	if (!m) goto translation_fail;
	*state |= m;
    }

    /* figure out the button */
    *button = atoi(l);
    if (!*button) {
	g_warning("Invalid button '%s' in pointer binding.", l);
	goto translation_fail;
    }

    ret = TRUE;

translation_fail:
    g_strfreev(parsed);
    return ret;
}

void grab_button(Client *client, guint state, guint button, GQuark context,
		 gboolean grab)
{
    Window win;
    int mode = GrabModeAsync;
    unsigned int mask;

    if (context == g_quark_try_string("frame")) {
	win = client->frame->window;
	mask = ButtonPressMask | ButtonMotionMask | ButtonReleaseMask;
    } else if (context == g_quark_try_string("client")) {
	win = client->frame->plate;
	mode = GrabModeSync; /* this is handled in mbind_fire */
	mask = ButtonPressMask; /* can't catch more than this with Sync mode
				   the release event is manufactured in
				   mbind_fire */
    } else return;

    if (grab)
	XGrabButton(ob_display, button, state, win, FALSE, mask, mode,
		    GrabModeAsync, None, None);
    else
	XUngrabButton(ob_display, button, state, win);
}

static void mbind_foreach_grab(GQuark key, gpointer data, gpointer user_data)
{
    struct mbind_foreach_grab_temp *d = user_data;
    PointerBinding *b = ((GSList *)data)->data;
    if (b != NULL)
 	grab_button(d->client, b->state, b->button, key, d->grab);
}
  
void mbind_grab_all(Client *client, gboolean grab)
{
    struct mbind_foreach_grab_temp bt;
    bt.client = client;
    bt.grab = grab;
    g_datalist_foreach(&bound_contexts, mbind_foreach_grab, &bt);
}

void grab_all_clients(gboolean grab)
{
    GSList *it;

    for (it = client_list; it != NULL; it = it->next)
	mbind_grab_all(it->data, grab);
}

void mbind_startup()
{
    grabbed = FALSE;
    g_datalist_init(&bound_contexts);
}

void mbind_shutdown()
{
    if (grabbed)
	mbind_grab_pointer(FALSE);
    mbind_clearall();
    g_datalist_clear(&bound_contexts);
}

gboolean mbind_add(char *name, GQuark context)
{
    guint state, button;
    PointerBinding *b;
    GSList *it;

    if (!translate(name, &state, &button))
	return FALSE;

    for (it = g_datalist_id_get_data(&bound_contexts, context);
	 it != NULL; it = it->next){
	b = it->data;
	if (b->state == state && b->button == button)
	    return TRUE; /* already bound */
    }

    grab_all_clients(FALSE);

    /* add the binding */
    b = g_new(PointerBinding, 1);
    b->state = state;
    b->button = button;
    b->name = g_strdup(name);
    g_datalist_id_set_data(&bound_contexts, context, 
        g_slist_append(g_datalist_id_get_data(&bound_contexts, context), b));
    grab_all_clients(TRUE);

    return TRUE;
}

static void mbind_foreach_clear(GQuark key, gpointer data, gpointer user_data)
{
    GSList *it;
    user_data = user_data;
    for (it = data; it != NULL; it = it->next) {
        PointerBinding *b = it->data;
        g_free(b->name);
        g_free(b);
    }
    g_slist_free(data);
}
void mbind_clearall()
{
    grab_all_clients(FALSE);
    g_datalist_foreach(&bound_contexts, mbind_foreach_clear, NULL);
}

void mbind_fire(guint state, guint button, GQuark context, EventType type,
		Client *client, int xroot, int yroot)
{
    GSList *it;

    if (grabbed) {
	    EventData *data;
	    data = eventdata_new_pointer(type, context, client, state, button,
					 NULL, xroot, yroot);
	    g_assert(data != NULL);
	    hooks_fire_pointer(data);
	    eventdata_free(data);
	    return;
    }

    for (it = g_datalist_id_get_data(&bound_contexts, context);
	 it != NULL; it = it->next){
	PointerBinding *b = it->data;
	if (b->state == state && b->button == button) {
	    EventData *data;
	    data = eventdata_new_pointer(type, context, client, state, button,
					 b->name, xroot, yroot);
	    g_assert(data != NULL);
	    hooks_fire(data);
	    eventdata_free(data);
	    break;
	}
    }
}

gboolean mbind_grab_pointer(gboolean grab)
{
    gboolean ret = TRUE;
    if (grab)
	ret = XGrabPointer(ob_display, ob_root, FALSE, (ButtonPressMask |
							ButtonReleaseMask |
							ButtonMotionMask |
							PointerMotionMask),
			   GrabModeAsync, GrabModeAsync, None, None,
			   CurrentTime) == GrabSuccess;
    else
	XUngrabPointer(ob_display, CurrentTime);
    return ret;
}
