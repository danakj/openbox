#include "../../kernel/openbox.h"
#include "../../kernel/dispatch.h"
#include "../../kernel/action.h"
#include "../../kernel/client.h"
#include "../../kernel/frame.h"
#include "../../kernel/engine.h"
#include "translate.h"
#include "mouse.h"
#include <glib.h>

/* GData of GSList*s of PointerBinding*s. */
static GData *bound_contexts;

struct foreach_grab_temp {
    Client *client;
    gboolean grab;
};

static void grab_button(Client *client, guint state, guint button,
			GQuark context, gboolean grab)
{
    Window win;
    int mode = GrabModeAsync;
    unsigned int mask;

    if (context == g_quark_try_string("frame")) {
	win = client->frame->window;
	mask = ButtonPressMask | ButtonMotionMask | ButtonReleaseMask;
    } else if (context == g_quark_try_string("client")) {
	win = client->frame->plate;
	mode = GrabModeSync; /* this is handled in pointer_event */
	mask = ButtonPressMask; /* can't catch more than this with Sync mode
				   the release event is manufactured in
				   pointer_fire */
    } else return;

    if (grab)
        /* XXX grab all lock keys */
	XGrabButton(ob_display, button, state, win, FALSE, mask, mode,
		    GrabModeAsync, None, None);
    else
        /* XXX ungrab all lock keys */
	XUngrabButton(ob_display, button, state, win);
}

static void foreach_grab(GQuark key, gpointer data, gpointer user_data)
{
    struct foreach_grab_temp *d = user_data;
    GSList *it;
    for (it = data; it != NULL; it = it->next) {
	MouseBinding *b = it->data;
 	grab_button(d->client, b->state, b->button, key, d->grab);
    }
}
  
static void grab_for_client(Client *client, gboolean grab)
{
    struct foreach_grab_temp bt;
    bt.client = client;
    bt.grab = grab;
    g_datalist_foreach(&bound_contexts, foreach_grab, &bt);
}

static void grab_all_clients(gboolean grab)
{
    GSList *it;

    for (it = client_list; it != NULL; it = it->next)
	grab_for_client(it->data, grab);
}

static void foreach_clear(GQuark key, gpointer data, gpointer user_data)
{
    GSList *it;
    user_data = user_data;
    for (it = data; it != NULL; it = it->next) {
	int i;

        MouseBinding *b = it->data;
	for (i = 0; i < NUM_MOUSEACTION; ++i)
            if (b->action[i] != NULL)
                action_free(b->action[i]);
        g_free(b);
    }
    g_slist_free(data);
}

static void fire_button(MouseAction a, GQuark context, Client *c, guint state,
                        guint button)
{
    GSList *it;
    MouseBinding *b;

    for (it = g_datalist_id_get_data(&bound_contexts, context);
         it != NULL; it = it->next) {
        b = it->data;
        if (b->state == state && b->button == button)
            break;
    }
    /* if not bound, then nothing to do! */
    if (it == NULL) return;

    if (b->action[a] != NULL && b->action[a]->func != NULL) {
        b->action[a]->data.any.c = c;

        g_assert(!(b->action[a]->func == action_move ||
                   b->action[a]->func == action_resize));

        b->action[a]->func(&b->action[a]->data);
    }
}

/* corner should be the opposite corner of the window in which the pointer
   clicked, Corner_TopLeft if a good default if there is no client */
static void fire_motion(MouseAction a, GQuark context, Client *c, guint state,
                        guint button, int cx, int cy, int cw, int ch,
                        int dx, int dy, gboolean final, Corner corner)
{
    GSList *it;
    MouseBinding *b;

    for (it = g_datalist_id_get_data(&bound_contexts, context);
         it != NULL; it = it->next) {
        b = it->data;
        if (b->state == state && b->button == button)
		break;
    }
    /* if not bound, then nothing to do! */
    if (it == NULL) return;

    if (b->action[a] != NULL && b->action[a]->func != NULL) {
        b->action[a]->data.any.c = c;

        if (b->action[a]->func == action_move) {
            b->action[a]->data.move.x = cx + dx;
            b->action[a]->data.move.y = cy + dy;
            b->action[a]->data.move.final = final;
        } else if (b->action[a]->func == action_resize) {
            b->action[a]->data.resize.corner = corner;
            switch (corner) {
            case Corner_TopLeft:
                b->action[a]->data.resize.x = cw + dx;
                b->action[a]->data.resize.y = ch + dy;
                break;
            case Corner_TopRight:
                b->action[a]->data.resize.x = cw - dx;
                b->action[a]->data.resize.y = ch + dy;
                break;
            case Corner_BottomLeft:
                b->action[a]->data.resize.x = cw + dx;
                b->action[a]->data.resize.y = ch - dy;
                break;
            case Corner_BottomRight:
                b->action[a]->data.resize.x = cw - dx;
                b->action[a]->data.resize.y = ch - dy;
                break;
            }
            b->action[a]->data.resize.final = final;
        }
        b->action[a]->func(&b->action[a]->data);
    }
}

static Corner pick_corner(int x, int y, int cx, int cy, int cw, int ch)
{
    if (x - cx < cw / 2) {
        if (y - cy < ch / 2)
            return Corner_BottomRight;
        else
            return Corner_TopRight;
    } else {
        if (y - cy < ch / 2)
            return Corner_BottomLeft;
        else
            return Corner_TopLeft;
    }
}

static void event(ObEvent *e, void *foo)
{
    static Time ltime;
    static int px, py, cx, cy, cw, ch, dx, dy;
    static guint button = 0, lbutton = 0;
    static gboolean drag = FALSE;
    static Corner corner = Corner_TopLeft;
    gboolean click = FALSE;
    gboolean dclick = FALSE;

    switch (e->type) {
    case Event_Client_Mapped:
        grab_for_client(e->data.c.client, TRUE);
        break;

    case Event_Client_Destroy:
        grab_for_client(e->data.c.client, FALSE);
        break;

    case Event_X_ButtonPress:
        if (!button) {
            if (e->data.x.client != NULL) {
                cx = e->data.x.client->frame->area.x;
                cy = e->data.x.client->frame->area.y;
                cw = e->data.x.client->frame->area.width;
                ch = e->data.x.client->frame->area.height;
                px = e->data.x.e->xbutton.x_root;
                py = e->data.x.e->xbutton.y_root;
                corner = pick_corner(px, py, cx, cy, cw, ch);
            }
            button = e->data.x.e->xbutton.button;
        }
        fire_button(MouseAction_Press,
                    engine_get_context(e->data.x.client,
                                       e->data.x.e->xbutton.window),
                    e->data.x.client, e->data.x.e->xbutton.state,
                    e->data.x.e->xbutton.button);
        break;

    case Event_X_ButtonRelease:
        if (e->data.x.e->xbutton.button == button) {
            /* end drags */
            if (drag) {
                fire_motion(MouseAction_Motion,
                            engine_get_context(e->data.x.client,
                                               e->data.x.e->xbutton.window),
                            e->data.x.client, e->data.x.e->xbutton.state,
                            e->data.x.e->xbutton.button,
                            cx, cy, cw, ch, dx, dy, TRUE, corner);
                drag = FALSE;
                
                lbutton = 0;
            } else {
                /* clicks are only valid if its released over the window */
                if (e->data.x.e->xbutton.x >= 0 &&
                    e->data.x.e->xbutton.y >= 0) {
                    int junk;
                    Window wjunk;
                    guint ujunk, w, h;
                    XGetGeometry(ob_display, e->data.x.e->xbutton.window,
                                 &wjunk, &junk, &junk, &w, &h, &ujunk, &ujunk);
                    if (e->data.x.e->xbutton.x < (signed)w &&
                        e->data.x.e->xbutton.y < (signed)h) {
                        click =TRUE;
                        /* double clicks happen if there were 2 in a row! */
                        if (lbutton == button &&
                            e->data.x.e->xbutton.time - 300 <= ltime)
                            dclick = TRUE;
                    }
                    lbutton = button;
                } else
                    lbutton = 0;
            }

            button = 0;
            ltime = e->data.x.e->xbutton.time;
        }
        fire_button(MouseAction_Press,
                    engine_get_context(e->data.x.client,
                                       e->data.x.e->xbutton.window),
                    e->data.x.client, e->data.x.e->xbutton.state,
                    e->data.x.e->xbutton.button);
        if (click)
            fire_button(MouseAction_Click,
                        engine_get_context(e->data.x.client,
                                           e->data.x.e->xbutton.window),
                        e->data.x.client, e->data.x.e->xbutton.state,
                        e->data.x.e->xbutton.button);
        if (dclick)
            fire_button(MouseAction_DClick,
                        engine_get_context(e->data.x.client,
                                           e->data.x.e->xbutton.window),
                        e->data.x.client, e->data.x.e->xbutton.state,
                        e->data.x.e->xbutton.button);
        break;

    case Event_X_MotionNotify:
        if (button) {
            drag = TRUE;
            dx = e->data.x.e->xmotion.x_root - px;
            dy = e->data.x.e->xmotion.y_root - py;
            fire_motion(MouseAction_Motion,
                        engine_get_context(e->data.x.client,
                                           e->data.x.e->xbutton.window),
                        e->data.x.client, e->data.x.e->xmotion.state,
                        button, cx, cy, cw, ch, dx, dy, FALSE, corner);
        }
        break;

    default:
        g_assert_not_reached();
    }
}

static gboolean mbind(char *buttonstr, char *contextstr, MouseAction mact,
                      Action *action)
{
    guint state, button;
    GQuark context;
    MouseBinding *b;
    GSList *it;
    guint i;

    if (!translate_button(buttonstr, &state, &button)) {
        g_warning("invalid button");
        return FALSE;
    }

    context = g_quark_try_string(contextstr);
    if (!context) {
        g_warning("invalid context");
        return FALSE;
    }

    for (it = g_datalist_id_get_data(&bound_contexts, context);
	 it != NULL; it = it->next){
	b = it->data;
	if (b->state == state && b->button == button) {
	    /* already bound */
            if (b->action[mact] != NULL) {
                g_warning("duplicate binding");
                return FALSE;
            }
            b->action[mact] = action;
            return TRUE;
	}
    }

    grab_all_clients(FALSE);

    /* add the binding */
    b = g_new(MouseBinding, 1);
    b->state = state;
    b->button = button;
    for (i = 0; i < NUM_MOUSEACTION; ++i)
        b->action[i] = NULL;
    b->action[mact] = action;
    g_datalist_id_set_data(&bound_contexts, context, 
        g_slist_append(g_datalist_id_get_data(&bound_contexts, context), b));

    grab_all_clients(TRUE);

    return TRUE;
}

static void binddef()
{
    Action *a;

    /* When creating an Action struct, all of the data elements in the
       appropriate struct need to be set, except the Client*, which will be set
       at call-time when then action function is used.

       For action_move and action_resize, the 'x', 'y', and 'final' data
       elements should not be set, as they are set at call-time.       
    */

    a = action_new(action_move);
    mbind("1", "titlebar", MouseAction_Motion, a);
    a = action_new(action_move);
    mbind("1", "handle", MouseAction_Motion, a);
    a = action_new(action_move);
    mbind("A-1", "frame", MouseAction_Motion, a);

    a = action_new(action_resize);
    mbind("1", "blcorner", MouseAction_Motion, a);
    a = action_new(action_resize);
    mbind("1", "brcorner", MouseAction_Motion, a);
    a = action_new(action_resize);
    mbind("A-3", "frame", MouseAction_Motion, a);

    a = action_new(action_raise);
    mbind("1", "titlebar", MouseAction_Press, a);
    a = action_new(action_raise);
    mbind("1", "handle", MouseAction_Press, a);
    a = action_new(action_lower);
    mbind("2", "titlebar", MouseAction_Press, a);
    a = action_new(action_lower);
    mbind("2", "handle", MouseAction_Press, a);
    a = action_new(action_raise);
    mbind("A-1", "frame", MouseAction_Click, a);
    a = action_new(action_lower);
    mbind("A-3", "frame", MouseAction_Click, a);

    a = action_new(action_toggle_shade);
    mbind("1", "titlebar", MouseAction_DClick, a);
    a = action_new(action_shade);
    mbind("4", "titlebar", MouseAction_Press, a);
    a = action_new(action_unshade);
    mbind("5", "titlebar", MouseAction_Click, a);

    a = action_new(action_toggle_maximize_full);
    mbind("1", "maximize", MouseAction_Click, a);
    a = action_new(action_toggle_maximize_vert);
    mbind("2", "maximize", MouseAction_Click, a);
    a = action_new(action_toggle_maximize_horz);
    mbind("3", "maximize", MouseAction_Click, a);
    a = action_new(action_iconify);
    mbind("1", "iconify", MouseAction_Click, a);
    a = action_new(action_close);
    mbind("1", "icon", MouseAction_DClick, a);
    a = action_new(action_close);
    mbind("1", "close", MouseAction_Click, a);
    a = action_new(action_toggle_omnipresent);
    mbind("1", "alldesktops", MouseAction_Click, a);

    a = action_new(action_next_desktop);
    a->data.nextprevdesktop.wrap = TRUE;
    mbind("4", "root", MouseAction_Click, a);
    a = action_new(action_previous_desktop);
    a->data.nextprevdesktop.wrap = TRUE;
    mbind("5", "root", MouseAction_Click, a);
    a = action_new(action_next_desktop);
    a->data.nextprevdesktop.wrap = TRUE;
    mbind("A-4", "root", MouseAction_Click, a);
    a = action_new(action_previous_desktop);
    a->data.nextprevdesktop.wrap = TRUE;
    mbind("A-5", "root", MouseAction_Click, a);
    a = action_new(action_next_desktop);
    a->data.nextprevdesktop.wrap = TRUE;
    mbind("A-4", "frame", MouseAction_Click, a);
    a = action_new(action_previous_desktop);
    a->data.nextprevdesktop.wrap = TRUE;
    mbind("A-5", "frame", MouseAction_Click, a);
}

void plugin_startup()
{
    dispatch_register(Event_Client_Mapped | Event_Client_Destroy |
                      Event_X_ButtonPress | Event_X_ButtonRelease |
                      Event_X_MotionNotify, (EventHandler)event, NULL);

    /* XXX parse a config */
    binddef();
}

void plugin_shutdown()
{
    dispatch_register(0, (EventHandler)event, NULL);

    grab_all_clients(FALSE);
    g_datalist_foreach(&bound_contexts, foreach_clear, NULL);
}
