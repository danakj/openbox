#include "kernel/openbox.h"
#include "kernel/dispatch.h"
#include "kernel/action.h"
#include "kernel/event.h"
#include "kernel/client.h"
#include "kernel/prop.h"
#include "kernel/grab.h"
#include "kernel/parse.h"
#include "kernel/frame.h"
#include "translate.h"
#include "mouse.h"
#include "mouseparse.h"
#include <glib.h>

static int threshold;
static int dclicktime;

static void parse_assign(char *name, ParseToken *value)
{
    if (!g_ascii_strcasecmp(name, "dragthreshold")) {
        if (value->type != TOKEN_INTEGER)
            yyerror("invalid value");
        else {
            if (value->data.integer >= 0)
                threshold = value->data.integer;
        }
    } else if (!g_ascii_strcasecmp(name, "doubleclicktime")) {
        if (value->type != TOKEN_INTEGER)
            yyerror("invalid value");
        else {
            if (value->data.integer >= 0)
                dclicktime = value->data.integer;
        }
    } else
        yyerror("invalid option");
    parse_free_token(value);
}

void plugin_setup_config()
{
    threshold = 3;
    dclicktime = 200;
    parse_reg_section("mouse", mouseparse, parse_assign);
}

/* Array of GSList*s of PointerBinding*s. */
static GSList *bound_contexts[NUM_CONTEXTS];

static void grab_for_client(Client *client, gboolean grab)
{
    int i;
    GSList *it;

    for (i = 0; i < NUM_CONTEXTS; ++i)
        for (it = bound_contexts[i]; it != NULL; it = it->next) {
            /* grab/ungrab the button */
            MouseBinding *b = it->data;
            Window win;
            int mode;
            unsigned int mask;

            if (i == Context_Frame) {
                win = client->frame->window;
                mode = GrabModeAsync;
                mask = ButtonPressMask | ButtonMotionMask | ButtonReleaseMask;
            } else if (i == Context_Client) {
                win = client->frame->plate;
                mode = GrabModeSync; /* this is handled in event */
                mask = ButtonPressMask; /* can't catch more than this with Sync
                                           mode the release event is
                                           manufactured in event() */
            } else continue;

            if (grab)
                grab_button(b->button, b->state, win, mask, mode);
            else
                ungrab_button(b->button, b->state, win);
        }
}

static void grab_all_clients(gboolean grab)
{
    GList *it;

    for (it = client_list; it != NULL; it = it->next)
	grab_for_client(it->data, grab);
}

static void clearall()
{
    int i;
    GSList *it;
    
    for(i = 0; i < NUM_CONTEXTS; ++i) {
        for (it = bound_contexts[i]; it != NULL; it = it->next) {
            int j;

            MouseBinding *b = it->data;
            for (j = 0; j < NUM_MOUSEACTION; ++j)
                if (b->action[j] != NULL)
                    action_free(b->action[j]);
            g_free(b);
        }
        g_slist_free(bound_contexts[i]);
    }
}

static void fire_button(MouseAction a, Context context, Client *c, guint state,
                        guint button, int x, int y)
{
    GSList *it;
    MouseBinding *b;

    for (it = bound_contexts[context]; it != NULL; it = it->next) {
        b = it->data;
        if (b->state == state && b->button == button)
            break;
    }
    /* if not bound, then nothing to do! */
    if (it == NULL) return;

    if (b->action[a] != NULL && b->action[a]->func != NULL) {
        b->action[a]->data.any.c = c;

        g_assert(b->action[a]->func != action_moveresize);

        if (b->action[a]->func == action_showmenu) {
            b->action[a]->data.showmenu.x = x;
            b->action[a]->data.showmenu.y = y;
        }

        b->action[a]->func(&b->action[a]->data);
    }
}

static void fire_motion(MouseAction a, Context context, Client *c,
                        guint state, guint button, int x_root, int y_root,
                        guint32 corner)
{
    GSList *it;
    MouseBinding *b;

    for (it = bound_contexts[context]; it != NULL; it = it->next) {
        b = it->data;
        if (b->state == state && b->button == button)
		break;
    }
    /* if not bound, then nothing to do! */
    if (it == NULL) return;

    if (b->action[a] != NULL && b->action[a]->func != NULL) {
        b->action[a]->data.any.c = c;

        if (b->action[a]->func == action_moveresize) {
            b->action[a]->data.moveresize.x = x_root;
            b->action[a]->data.moveresize.y = y_root;
            b->action[a]->data.moveresize.button = button;
            if (!(b->action[a]->data.moveresize.corner ==
                  prop_atoms.net_wm_moveresize_move ||
                  b->action[a]->data.moveresize.corner ==
                  prop_atoms.net_wm_moveresize_move_keyboard ||
                  b->action[a]->data.moveresize.corner ==
                  prop_atoms.net_wm_moveresize_size_keyboard))
                b->action[a]->data.moveresize.corner = corner;
        } else
            g_assert_not_reached();

        b->action[a]->func(&b->action[a]->data);
    }
}

static guint32 pick_corner(int x, int y, int cx, int cy, int cw, int ch)
{
    if (x - cx < cw / 2) {
        if (y - cy < ch / 2)
            return prop_atoms.net_wm_moveresize_size_topleft;
        else
            return prop_atoms.net_wm_moveresize_size_bottomleft;
    } else {
        if (y - cy < ch / 2)
            return prop_atoms.net_wm_moveresize_size_topright;
        else
            return prop_atoms.net_wm_moveresize_size_bottomright;
    }
}

static void event(ObEvent *e, void *foo)
{
    static Time ltime;
    static guint button = 0, state = 0, lbutton = 0;
    static int px, py;
    gboolean click = FALSE;
    gboolean dclick = FALSE;
    Context context;
    
    switch (e->type) {
    case Event_Client_Mapped:
        grab_for_client(e->data.c.client, TRUE);
        break;

    case Event_Client_Destroy:
        grab_for_client(e->data.c.client, FALSE);
        break;

    case Event_X_ButtonPress:
        context = frame_context(e->data.x.client,
                                e->data.x.e->xbutton.window);

        if (!button) {
            px = e->data.x.e->xbutton.x_root;
            py = e->data.x.e->xbutton.y_root;
            button = e->data.x.e->xbutton.button;
            state = e->data.x.e->xbutton.state;
        }

        fire_button(MouseAction_Press, context,
                    e->data.x.client, e->data.x.e->xbutton.state,
                    e->data.x.e->xbutton.button,
                    e->data.x.e->xbutton.x_root, e->data.x.e->xbutton.y_root);

        if (context == Context_Client) {
            /* Replay the event, so it goes to the client*/
            XAllowEvents(ob_display, ReplayPointer, event_lasttime);
            /* Fall through to the release case! */
        } else
            break;

    case Event_X_ButtonRelease:
        context = frame_context(e->data.x.client,
                                e->data.x.e->xbutton.window);
        if (e->data.x.e->xbutton.button == button) {
            /* clicks are only valid if its released over the window */
            int junk;
            Window wjunk;
            guint ujunk, b, w, h;
            XGetGeometry(ob_display, e->data.x.e->xbutton.window,
                         &wjunk, &junk, &junk, &w, &h, &b, &ujunk);
            if (e->data.x.e->xbutton.x >= (signed)-b &&
                e->data.x.e->xbutton.y >= (signed)-b &&
                e->data.x.e->xbutton.x < (signed)(w+b) &&
                e->data.x.e->xbutton.y < (signed)(h+b)) {
                click = TRUE;
                /* double clicks happen if there were 2 in a row! */
                if (lbutton == button &&
                    e->data.x.e->xbutton.time - dclicktime <= ltime) {
                    dclick = TRUE;
                    lbutton = 0;
                } else
                    lbutton = button;
            } else
                lbutton = 0;

            button = 0;
            state = 0;
            ltime = e->data.x.e->xbutton.time;
        }
        fire_button(MouseAction_Release, context,
                    e->data.x.client, e->data.x.e->xbutton.state,
                    e->data.x.e->xbutton.button,
                    e->data.x.e->xbutton.x_root, e->data.x.e->xbutton.y_root);
        if (click)
            fire_button(MouseAction_Click, context,
                        e->data.x.client, e->data.x.e->xbutton.state,
                        e->data.x.e->xbutton.button,
                        e->data.x.e->xbutton.x_root,
                        e->data.x.e->xbutton.y_root);
        if (dclick)
            fire_button(MouseAction_DClick, context,
                        e->data.x.client, e->data.x.e->xbutton.state,
                        e->data.x.e->xbutton.button,
                        e->data.x.e->xbutton.x_root,
                        e->data.x.e->xbutton.y_root);
        break;

    case Event_X_MotionNotify:
        if (button) {
            if (ABS(e->data.x.e->xmotion.x_root - px) >= threshold ||
                ABS(e->data.x.e->xmotion.y_root - py) >= threshold) {
                guint32 corner;

                if (!client)
                    corner = prop_atoms.net_am_moveresize_size_bottomright;
                else
                    corner =
                        pick_corner(e->data.x.e->xmotion.x_root,
                                    e->data.x.e->xmotion.y_root,
                                    e->data.x.client->frame->area.x,
                                    e->data.x.client->frame->area.y,
                                    /* use the client size because the frame
                                       can be differently sized (shaded
                                       windows) and we want this based on the
                                       clients size */
                                    e->data.x.client->area.width +
                                    e->data.x.client->frame->size.left +
                                    e->data.x.client->frame->size.right,
                                    e->data.x.client->area.height +
                                    e->data.x.client->frame->size.top +
                                    e->data.x.client->frame->size.bottom);
                context = frame_context(e->data.x.client,
                                        e->data.x.e->xmotion.window);
                fire_motion(MouseAction_Motion, context,
                            e->data.x.client, state, button,
                            e->data.x.e->xmotion.x_root, 
                            e->data.x.e->xmotion.y_root, corner);
                button = 0;
                state = 0;
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
    Context context;
    MouseBinding *b;
    GSList *it;

    if (!translate_button(buttonstr, &state, &button)) {
        g_warning("invalid button '%s'", buttonstr);
        return FALSE;
    }

    contextstr = g_ascii_strdown(contextstr, -1);
    context = frame_context_from_string(contextstr);
    if (!context) {
        g_warning("invalid context '%s'", contextstr);
        g_free(contextstr);
        return FALSE;
    }
    g_free(contextstr);

    for (it = bound_contexts[context]; it != NULL; it = it->next){
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
    bound_contexts[context] = g_slist_append(bound_contexts[context], b);

    grab_all_clients(TRUE);

    return TRUE;
}

void plugin_startup()
{
    dispatch_register(Event_Client_Mapped | Event_Client_Destroy |
                      Event_X_ButtonPress | Event_X_ButtonRelease |
                      Event_X_MotionNotify, (EventHandler)event, NULL);
}

void plugin_shutdown()
{
    dispatch_register(0, (EventHandler)event, NULL);

    grab_all_clients(FALSE);
    clearall();
}
