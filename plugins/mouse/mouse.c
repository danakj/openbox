#include "../../kernel/openbox.h"
#include "../../kernel/dispatch.h"
#include "../../kernel/action.h"
#include "../../kernel/event.h"
#include "../../kernel/client.h"
#include "../../kernel/frame.h"
#include "../../kernel/grab.h"
#include "../../kernel/engine.h"
#include "translate.h"
#include "mouse.h"
#include "mouserc_parse.h"
#include <glib.h>

void plugin_setup_config()
{
}

static int drag_threshold = 3;

/* GData of GSList*s of PointerBinding*s. */
static GData *bound_contexts;

struct foreach_grab_temp {
    Client *client;
    gboolean grab;
};

static void foreach_grab(GQuark key, gpointer data, gpointer user_data)
{
    struct foreach_grab_temp *d = user_data;
    GSList *it;
    for (it = data; it != NULL; it = it->next) {
        /* grab/ungrab the button */
	MouseBinding *b = it->data;
        Window win;
        int mode;
        unsigned int mask;

        if (key == g_quark_try_string("frame")) {
            win = d->client->frame->window;
            mode = GrabModeAsync;
            mask = ButtonPressMask | ButtonMotionMask | ButtonReleaseMask;
        } else if (key == g_quark_try_string("client")) {
            win = d->client->frame->plate;
            mode = GrabModeSync; /* this is handled in event */
            mask = ButtonPressMask; /* can't catch more than this with Sync
                                       mode the release event is manufactured
                                       in event */
        } else return;

        if (d->grab)
            grab_button(b->button, b->state, win, mask, mode);
        else
            ungrab_button(b->button, b->state, win);
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
    GQuark context;

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
        context = engine_get_context(e->data.x.client,
                                     e->data.x.e->xbutton.window);

        fire_button(MouseAction_Press, context,
                    e->data.x.client, e->data.x.e->xbutton.state,
                    e->data.x.e->xbutton.button);

        if (context == g_quark_try_string("client")) {
            /* Replay the event, so it goes to the client*/
            XAllowEvents(ob_display, ReplayPointer, event_lasttime);
            /* Fall through to the release case! */
        } else
            break;

    case Event_X_ButtonRelease:
        context = engine_get_context(e->data.x.client,
                                     e->data.x.e->xbutton.window);
        if (e->data.x.e->xbutton.button == button) {
            /* end drags */
            if (drag) {
                fire_motion(MouseAction_Motion, context,
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
        fire_button(MouseAction_Press, context,
                    e->data.x.client, e->data.x.e->xbutton.state,
                    e->data.x.e->xbutton.button);
        if (click)
            fire_button(MouseAction_Click, context,
                        e->data.x.client, e->data.x.e->xbutton.state,
                        e->data.x.e->xbutton.button);
        if (dclick)
            fire_button(MouseAction_DClick, context,
                        e->data.x.client, e->data.x.e->xbutton.state,
                        e->data.x.e->xbutton.button);
        break;

    case Event_X_MotionNotify:
        if (button) {
            dx = e->data.x.e->xmotion.x_root - px;
            dy = e->data.x.e->xmotion.y_root - py;
            if (ABS(dx) >= drag_threshold || ABS(dy) >= drag_threshold)
                drag = TRUE;
            if (drag) {
                context = engine_get_context(e->data.x.client,
                                             e->data.x.e->xbutton.window);
                fire_motion(MouseAction_Motion, context,
                            e->data.x.client, e->data.x.e->xmotion.state,
                            button, cx, cy, cw, ch, dx, dy, FALSE, corner);
            }
        }
        break;

    default:
        g_assert_not_reached();
    }
}

gboolean mbind(char *buttonstr, char *contextstr, MouseAction mact,
               Action *action)
{
    guint state, button;
    GQuark context;
    MouseBinding *b;
    GSList *it;

    if (!translate_button(buttonstr, &state, &button)) {
        g_warning("invalid button '%s'", buttonstr);
        return FALSE;
    }

    contextstr = g_ascii_strdown(contextstr, -1);
    context = g_quark_try_string(contextstr);
    if (!context) {
        g_warning("invalid context '%s'", contextstr);
        g_free(contextstr);
        return FALSE;
    }
    g_free(contextstr);

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
    b = g_new0(MouseBinding, 1);
    b->state = state;
    b->button = button;
    b->action[mact] = action;
    g_datalist_id_set_data(&bound_contexts, context, 
        g_slist_append(g_datalist_id_get_data(&bound_contexts, context), b));

    grab_all_clients(TRUE);

    return TRUE;
}

void plugin_startup()
{
    dispatch_register(Event_Client_Mapped | Event_Client_Destroy |
                      Event_X_ButtonPress | Event_X_ButtonRelease |
                      Event_X_MotionNotify, (EventHandler)event, NULL);

    mouserc_parse();
}

void plugin_shutdown()
{
    dispatch_register(0, (EventHandler)event, NULL);

    grab_all_clients(FALSE);
    g_datalist_foreach(&bound_contexts, foreach_clear, NULL);
}
