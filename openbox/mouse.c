#include "openbox.h"
#include "config.h"
#include "action.h"
#include "event.h"
#include "client.h"
#include "prop.h"
#include "grab.h"
#include "frame.h"
#include "translate.h"
#include "mouse.h"
#include "keyboard.h"
#include <glib.h>

typedef struct {
    guint state;
    guint button;
    GSList *actions[OB_MOUSE_NUM_ACTIONS]; /* lists of Action pointers */
} ObMouseBinding;

/* Array of GSList*s of PointerBinding*s. */
static GSList *bound_contexts[OB_FRAME_NUM_CONTEXTS];

void mouse_grab_for_client(ObClient *client, gboolean grab)
{
    int i;
    GSList *it;

    for (i = 0; i < OB_FRAME_NUM_CONTEXTS; ++i)
        for (it = bound_contexts[i]; it != NULL; it = it->next) {
            /* grab/ungrab the button */
            ObMouseBinding *b = it->data;
            Window win;
            int mode;
            unsigned int mask;

            if (i == OB_FRAME_CONTEXT_FRAME) {
                win = client->frame->window;
                mode = GrabModeAsync;
                mask = ButtonPressMask | ButtonMotionMask | ButtonReleaseMask;
            } else if (i == OB_FRAME_CONTEXT_CLIENT) {
                win = client->frame->plate;
                mode = GrabModeSync; /* this is handled in event */
                mask = ButtonPressMask; /* can't catch more than this with Sync
                                           mode the release event is
                                           manufactured in event() */
            } else continue;

            if (grab)
                grab_button_full(b->button, b->state, win, mask, mode, None);
            else
                ungrab_button(b->button, b->state, win);
        }
}

static void grab_all_clients(gboolean grab)
{
    GList *it;

    for (it = client_list; it != NULL; it = it->next)
	mouse_grab_for_client(it->data, grab);
}

static void clearall()
{
    int i;
    GSList *it;
    
    for(i = 0; i < OB_FRAME_NUM_CONTEXTS; ++i) {
        for (it = bound_contexts[i]; it != NULL; it = it->next) {
            int j;

            ObMouseBinding *b = it->data;
            for (j = 0; j < OB_MOUSE_NUM_ACTIONS; ++j) {
                GSList *it;
                for (it = b->actions[j]; it; it = it->next) {
                    action_free(it->data);
                }
                g_slist_free(b->actions[j]);
            }
            g_free(b);
        }
        g_slist_free(bound_contexts[i]);
    }
}

static void fire_button(ObMouseAction a, ObFrameContext context,
                        ObClient *c, guint state,
                        guint button, int x, int y)
{
    GSList *it;
    ObMouseBinding *b;

    for (it = bound_contexts[context]; it != NULL; it = it->next) {
        b = it->data;
        if (b->state == state && b->button == button)
            break;
    }
    /* if not bound, then nothing to do! */
    if (it == NULL) return;

    for (it = b->actions[a]; it; it = it->next) {
        ObAction *act = it->data;
        if (act->func != NULL) {
            act->data.any.c = c;

            g_assert(act->func != action_moveresize);

            if (act->func == action_showmenu) {
                act->data.showmenu.x = x;
                act->data.showmenu.y = y;
            }

            if (act->func == action_desktop_dir)
            {
                act->data.desktopdir.final = FALSE;
                act->data.desktopdir.cancel = FALSE;
            }
            if (act->func == action_send_to_desktop_dir)
            {
                act->data.sendtodir.final = FALSE;
                act->data.sendtodir.cancel = FALSE;
            }

            if (config_desktop_popup &&
                (act->func == action_desktop_dir ||
                 act->func == action_send_to_desktop_dir))
            {
                keyboard_interactive_grab(state, c, context, act);
            }

            act->func(&act->data);
        }
    }
}

static void fire_motion(ObMouseAction a, ObFrameContext context, ObClient *c,
                        guint state, guint button, int x_root, int y_root,
                        guint32 corner)
{
    GSList *it;
    ObMouseBinding *b;

    for (it = bound_contexts[context]; it != NULL; it = it->next) {
        b = it->data;
        if (b->state == state && b->button == button)
		break;
    }
    /* if not bound, then nothing to do! */
    if (it == NULL) return;

    for (it = b->actions[a]; it; it = it->next) {
        ObAction *act = it->data;
        if (act->func != NULL) {
            act->data.any.c = c;

            if (act->func == action_moveresize) {
                act->data.moveresize.x = x_root;
                act->data.moveresize.y = y_root;
                act->data.moveresize.button = button;
                if (!(act->data.moveresize.corner ==
                      prop_atoms.net_wm_moveresize_move ||
                      act->data.moveresize.corner ==
                      prop_atoms.net_wm_moveresize_move_keyboard ||
                      act->data.moveresize.corner ==
                      prop_atoms.net_wm_moveresize_size_keyboard))
                    act->data.moveresize.corner = corner;
            } else
                g_assert_not_reached();

            act->func(&act->data);
        }
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

void mouse_event(ObClient *client, ObFrameContext context, XEvent *e)
{
    static Time ltime;
    static guint button = 0, state = 0, lbutton = 0;

    static Window lwindow = None;
    static int px, py;
    gboolean click = FALSE;
    gboolean dclick = FALSE;
    
    switch (e->type) {
    case ButtonPress:
        px = e->xbutton.x_root;
        py = e->xbutton.y_root;
        button = e->xbutton.button;
        state = e->xbutton.state;

        fire_button(OB_MOUSE_ACTION_PRESS, context,
                    client, e->xbutton.state,
                    e->xbutton.button,
                    e->xbutton.x_root, e->xbutton.y_root);

        if (context == OB_FRAME_CONTEXT_CLIENT) {
            /* Replay the event, so it goes to the client*/
            XAllowEvents(ob_display, ReplayPointer, event_lasttime);
            /* Fall through to the release case! */
        } else
            break;

    case ButtonRelease:
        if (e->xbutton.button == button) {
            /* clicks are only valid if its released over the window */
            int junk1, junk2;
            Window wjunk;
            guint ujunk, b, w, h;
            XGetGeometry(ob_display, e->xbutton.window,
                         &wjunk, &junk1, &junk2, &w, &h, &b, &ujunk);
            if (e->xbutton.x >= (signed)-b &&
                e->xbutton.y >= (signed)-b &&
                e->xbutton.x < (signed)(w+b) &&
                e->xbutton.y < (signed)(h+b)) {
                click = TRUE;
                /* double clicks happen if there were 2 in a row! */
                if (lbutton == button &&
                    lwindow == e->xbutton.window &&
                    e->xbutton.time - config_mouse_dclicktime <=
                    ltime) {
                    dclick = TRUE;
                    lbutton = 0;
                } else {
                    lbutton = button;
                    lwindow = e->xbutton.window;
                }
            } else {
                lbutton = 0;
                lwindow = None;
            }

            button = 0;
            state = 0;
            ltime = e->xbutton.time;
        }
        fire_button(OB_MOUSE_ACTION_RELEASE, context,
                    client, e->xbutton.state,
                    e->xbutton.button,
                    e->xbutton.x_root, e->xbutton.y_root);
        if (click)
            fire_button(OB_MOUSE_ACTION_CLICK, context,
                        client, e->xbutton.state,
                        e->xbutton.button,
                        e->xbutton.x_root,
                        e->xbutton.y_root);
        if (dclick)
            fire_button(OB_MOUSE_ACTION_DOUBLE_CLICK, context,
                        client, e->xbutton.state,
                        e->xbutton.button,
                        e->xbutton.x_root,
                        e->xbutton.y_root);
        break;

    case MotionNotify:
        if (button) {
            if (ABS(e->xmotion.x_root - px) >=
                config_mouse_threshold ||
                ABS(e->xmotion.y_root - py) >=
                config_mouse_threshold) {
                guint32 corner;

                /* You can't drag on buttons */
                if (context == OB_FRAME_CONTEXT_MAXIMIZE ||
                    context == OB_FRAME_CONTEXT_ALLDESKTOPS ||
                    context == OB_FRAME_CONTEXT_SHADE ||
                    context == OB_FRAME_CONTEXT_ICONIFY ||
                    context == OB_FRAME_CONTEXT_ICON ||
                    context == OB_FRAME_CONTEXT_CLOSE)
                    break;

                if (!client)
                    corner = prop_atoms.net_wm_moveresize_size_bottomright;
                else
                    corner =
                        pick_corner(e->xmotion.x_root,
                                    e->xmotion.y_root,
                                    client->frame->area.x,
                                    client->frame->area.y,
                                    /* use the client size because the frame
                                       can be differently sized (shaded
                                       windows) and we want this based on the
                                       clients size */
                                    client->area.width +
                                    client->frame->size.left +
                                    client->frame->size.right,
                                    client->area.height +
                                    client->frame->size.top +
                                    client->frame->size.bottom);
                fire_motion(OB_MOUSE_ACTION_MOTION, context,
                            client, state, button, px, py, corner);
                button = 0;
                state = 0;
            }
        }
        break;

    default:
        g_assert_not_reached();
    }
}

gboolean mouse_bind(char *buttonstr, char *contextstr, ObMouseAction mact,
                    ObAction *action)
{
    guint state, button;
    ObFrameContext context;
    ObMouseBinding *b;
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
            b->actions[mact] = g_slist_append(b->actions[mact], action);
            return TRUE;
	}
    }

    grab_all_clients(FALSE);

    /* add the binding */
    b = g_new0(ObMouseBinding, 1);
    b->state = state;
    b->button = button;
    b->actions[mact] = g_slist_append(NULL, action);
    bound_contexts[context] = g_slist_append(bound_contexts[context], b);

    grab_all_clients(TRUE);

    return TRUE;
}

void mouse_startup()
{
}

void mouse_shutdown()
{
    grab_all_clients(FALSE);
    clearall();
}
