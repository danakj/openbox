#include "openbox.h"
#include "client.h"
#include "xerror.h"
#include "prop.h"
#include "config.h"
#include "screen.h"
#include "frame.h"
#include "framerender.h"
#include "focus.h"
#include "stacking.h"
#include "extensions.h"
#include "timer.h"
#include "dispatch.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#ifdef HAVE_SYS_SELECT_H
#  include <sys/select.h>
#endif

static void event_process(XEvent *e);
static void event_handle_root(XEvent *e);
static void event_handle_client(Client *c, XEvent *e);

Time event_lasttime = 0;

/*! The value of the mask for the NumLock modifier */
unsigned int NumLockMask;
/*! The value of the mask for the ScrollLock modifier */
unsigned int ScrollLockMask;
/*! The key codes for the modifier keys */
static XModifierKeymap *modmap;
/*! Table of the constant modifier masks */
static const int mask_table[] = {
    ShiftMask, LockMask, ControlMask, Mod1Mask,
    Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
};
static int mask_table_size;

void event_startup()
{
    mask_table_size = sizeof(mask_table) / sizeof(mask_table[0]);
     
    /* get lock masks that are defined by the display (not constant) */
    modmap = XGetModifierMapping(ob_display);
    g_assert(modmap);
    if (modmap && modmap->max_keypermod > 0) {
	size_t cnt;
	const size_t size = mask_table_size * modmap->max_keypermod;
	/* get the values of the keyboard lock modifiers
	   Note: Caps lock is not retrieved the same way as Scroll and Num
	   lock since it doesn't need to be. */
	const KeyCode num_lock = XKeysymToKeycode(ob_display, XK_Num_Lock);
	const KeyCode scroll_lock = XKeysymToKeycode(ob_display,
						     XK_Scroll_Lock);
	  
	for (cnt = 0; cnt < size; ++cnt) {
	    if (! modmap->modifiermap[cnt]) continue;
	       
	    if (num_lock == modmap->modifiermap[cnt])
		NumLockMask = mask_table[cnt / modmap->max_keypermod];
	    if (scroll_lock == modmap->modifiermap[cnt])
		ScrollLockMask = mask_table[cnt / modmap->max_keypermod];
	}
    }
}

void event_shutdown()
{
    XFreeModifiermap(modmap);
}

void event_loop()
{
    fd_set selset;
    XEvent e;
    int x_fd;
    struct timeval *wait;

    while (TRUE) {
	/*
	  There are slightly different event retrieval semantics here for
	  local (or high bandwidth) versus remote (or low bandwidth)
	  connections to the display/Xserver.
	*/
	if (ob_remote) {
	    if (!XPending(ob_display))
		break;
	} else {
	    /*
	      This XSync allows for far more compression of events, which
	      makes things like Motion events perform far far better. Since
	      it also means network traffic for every event instead of every
	      X events (where X is the number retrieved at a time), it
	      probably should not be used for setups where Openbox is
	      running on a remote/low bandwidth display/Xserver.
	    */
	    XSync(ob_display, FALSE);
	    if (!XEventsQueued(ob_display, QueuedAlready))
		break;
	}
	XNextEvent(ob_display, &e);

	event_process(&e);
    }
     
    timer_dispatch((GTimeVal**)&wait);
    x_fd = ConnectionNumber(ob_display);
    FD_ZERO(&selset);
    FD_SET(x_fd, &selset);
    select(x_fd + 1, &selset, NULL, NULL, wait);
}

static Window event_get_window(XEvent *e)
{
    Window window;

    /* pick a window */
    switch (e->type) {
    case MapRequest:
	window = e->xmap.window;
	break;
    case UnmapNotify:
	window = e->xunmap.window;
	break;
    case DestroyNotify:
	window = e->xdestroywindow.window;
	break;
    case ConfigureRequest:
	window = e->xconfigurerequest.window;
	break;
    default:
#ifdef XKB
	if (extensions_xkb && e->type == extensions_xkb_event_basep) {
	    switch (((XkbAnyEvent*)&e)->xkb_type) {
	    case XkbBellNotify:
		window = ((XkbBellNotifyEvent*)&e)->window;
	    default:
		window = None;
	    }
        } else
#endif
            window = e->xany.window;
    }
    return window;
}

static void event_set_lasttime(XEvent *e)
{
    /* grab the lasttime and hack up the state */
    switch (e->type) {
    case ButtonPress:
    case ButtonRelease:
	event_lasttime = e->xbutton.time;
	break;
    case KeyPress:
	event_lasttime = e->xkey.time;
	break;
    case KeyRelease:
	event_lasttime = e->xkey.time;
	break;
    case MotionNotify:
	event_lasttime = e->xmotion.time;
	break;
    case PropertyNotify:
	event_lasttime = e->xproperty.time;
	break;
    case EnterNotify:
    case LeaveNotify:
	event_lasttime = e->xcrossing.time;
	break;
    default:
        event_lasttime = CurrentTime;
        break;
    }
}

#define STRIP_MODS(s) \
	s &= ~(LockMask | NumLockMask | ScrollLockMask), \
	/* kill off the Button1Mask etc, only want the modifiers */ \
	s &= (ControlMask | ShiftMask | Mod1Mask | \
              Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask) \

static void event_hack_mods(XEvent *e)
{
    KeyCode *kp;
    int i, k;

    switch (e->type) {
    case ButtonPress:
    case ButtonRelease:
        STRIP_MODS(e->xbutton.state);
	break;
    case KeyPress:
        STRIP_MODS(e->xkey.state);
	break;
    case KeyRelease:
        STRIP_MODS(e->xkey.state);
	/* remove from the state the mask of the modifier being released, if
	   it is a modifier key being released (this is a little ugly..) */
	kp = modmap->modifiermap;
	for (i = 0; i < mask_table_size; ++i) {
	    for (k = 0; k < modmap->max_keypermod; ++k) {
		if (*kp == e->xkey.keycode) { /* found the keycode */
		    /* remove the mask for it */
		    e->xkey.state &= ~mask_table[i];
		    /* cause the first loop to break; */
		    i = mask_table_size;
		    break; /* get outta here! */
		}
		++kp;
	    }
	}
	break;
    case MotionNotify:
        STRIP_MODS(e->xmotion.state);
	/* compress events */
        {
            XEvent ce;
            while (XCheckTypedWindowEvent(ob_display, e->xmotion.window,
                                          e->type, &ce)) {
                e->xmotion.x_root = ce.xmotion.x_root;
                e->xmotion.y_root = ce.xmotion.y_root;
            }
	}
	break;
    }
}

static gboolean event_ignore(XEvent *e, Client *client)
{
    switch(e->type) {
    case FocusIn:
#ifdef DEBUG_FOCUS
        g_message("FocusIn on %lx mode %d detail %d", window,
                  e->xfocus.mode, e->xfocus.detail);
#endif
        /* NotifyAncestor is not ignored in FocusIn like it is in FocusOut
           because of RevertToPointerRoot. If the focus ends up reverting to
           pointer root on a workspace change, then the FocusIn event that we
           want will be of type NotifyAncestor. This situation does not occur
           for FocusOut, so it is safely ignored there.
        */
	if (e->xfocus.detail == NotifyInferior ||
            e->xfocus.detail > NotifyNonlinearVirtual ||
            client == NULL) {
            /* says a client was not found for the event (or a valid FocusIn
               event was not found.
            */
            e->xfocus.window = None;
            return TRUE;
        }

#ifdef DEBUG_FOCUS
        g_message("FocusIn on %lx", window);
#endif
        break;
    case FocusOut:
#ifdef DEBUG_FOCUS
        g_message("FocusOut on %lx mode %d detail %d", window,
                  e->xfocus.mode, e->xfocus.detail);
#endif
	if (e->xfocus.mode == NotifyGrab ||
            e->xfocus.detail == NotifyInferior ||
            e->xfocus.detail == NotifyAncestor ||
            e->xfocus.detail > NotifyNonlinearVirtual) return TRUE;
 
#ifdef DEBUG_FOCUS
       g_message("FocusOut on %lx", window);
#endif
        /* Try process a FocusIn first, and if a legit one isn't found, then
           do the fallback shiznit. */
        {
            XEvent fi, fo;
            gboolean isfo = FALSE;

            if (XCheckTypedEvent(ob_display, FocusIn, &fi)) {
		event_process(&fi);

                /* when we have gotten a fi/fo pair, then see if there are any
                   more fo's coming. if there are, then don't fallback just yet
                */
                if ((isfo = XCheckTypedEvent(ob_display, FocusOut, &fo)))
                    XPutBackEvent(ob_display, &fo);

                /* secret magic way of event_process telling us that no client
                   was found for the FocusIn event. ^_^ */
                if (!isfo && fi.xfocus.window == None)
                    focus_fallback(Fallback_NoFocus);
                if (fi.xfocus.window == e->xfocus.window)
                    return TRUE;
            } else
                focus_fallback(Fallback_NoFocus);
        }
	break;
    case EnterNotify:
    case LeaveNotify:
        /* NotifyUngrab occurs when a mouse button is released and the event is
           caused, like when lowering a window */
        if (e->xcrossing.mode == NotifyGrab ||
            e->xcrossing.detail == NotifyInferior)
            return TRUE;
	break;
    }
    return FALSE;
}

static void event_process(XEvent *e)
{
    Window window;
    Client *client;

    window = event_get_window(e);
    client = g_hash_table_lookup(client_map, &window);
    event_set_lasttime(e);
    event_hack_mods(e);
    if (event_ignore(e, client))
        return;

    /* deal with it in the kernel */
    if (client)
	event_handle_client(client, e);
    else if (window == ob_root)
	event_handle_root(e);
    else if (e->type == MapRequest)
	client_manage(window);
    else if (e->type == ConfigureRequest) {
	/* unhandled configure requests must be used to configure the
	   window directly */
	XWindowChanges xwc;
	       
	xwc.x = e->xconfigurerequest.x;
	xwc.y = e->xconfigurerequest.y;
	xwc.width = e->xconfigurerequest.width;
	xwc.height = e->xconfigurerequest.height;
	xwc.border_width = e->xconfigurerequest.border_width;
	xwc.sibling = e->xconfigurerequest.above;
	xwc.stack_mode = e->xconfigurerequest.detail;
       
	/* we are not to be held responsible if someone sends us an
	   invalid request! */
	xerror_set_ignore(TRUE);
	XConfigureWindow(ob_display, window,
			 e->xconfigurerequest.value_mask, &xwc);
	xerror_set_ignore(FALSE);
    }

    /* user input (action-bound) events */
    /*
    if (e->type == ButtonPress || e->type == ButtonRelease ||
        e->type == MotionNotify)
        mouse_event(e, client);
    else if (e->type == KeyPress || e->type == KeyRelease)
        ;
    */

    /* dispatch the event to registered handlers */
    dispatch_x(e, client);
}

static void event_handle_root(XEvent *e)
{
    Atom msgtype;
     
    switch(e->type) {
    case ClientMessage:
	if (e->xclient.format != 32) break;

	msgtype = e->xclient.message_type;
	if (msgtype == prop_atoms.net_current_desktop) {
	    unsigned int d = e->xclient.data.l[0];
	    if (d < screen_num_desktops)
		screen_set_desktop(d);
	} else if (msgtype == prop_atoms.net_number_of_desktops) {
	    unsigned int d = e->xclient.data.l[0];
	    if (d > 0)
		screen_set_num_desktops(d);
	} else if (msgtype == prop_atoms.net_showing_desktop) {
	    screen_show_desktop(e->xclient.data.l[0] != 0);
	}
	break;
    case PropertyNotify:
	if (e->xproperty.atom == prop_atoms.net_desktop_names)
	    screen_update_desktop_names();
	else if (e->xproperty.atom == prop_atoms.net_desktop_layout)
	    screen_update_layout();
	break;
    }
}

static void event_handle_client(Client *client, XEvent *e)
{
    XEvent ce;
    Atom msgtype;
    int i=0;
     
    switch (e->type) {
    case ButtonPress:
    case ButtonRelease:
        switch (frame_context(client, e->xbutton.window)) {
        case Context_Maximize:
            client->frame->max_press = (e->type == ButtonPress);
            framerender_frame(client->frame);
            break;
        case Context_Close:
            client->frame->close_press = (e->type == ButtonPress);
            framerender_frame(client->frame);
            break;
        case Context_Iconify:
            client->frame->iconify_press = (e->type == ButtonPress);
            framerender_frame(client->frame);
            break;
        case Context_AllDesktops:
            client->frame->desk_press = (e->type == ButtonPress);
            framerender_frame(client->frame);
            break; 
        case Context_Shade:
            client->frame->shade_press = (e->type == ButtonPress);
            framerender_frame(client->frame);
            break;
        default:
            /* nothing changes with clicks for any other contexts */
            break;
        }
        break;
    case FocusIn:
        focus_set_client(client);
    case FocusOut:
#ifdef DEBUG_FOCUS
        g_message("Focus%s on client for %lx", (e->type==FocusIn?"In":"Out"),
                  client->window);
#endif
        /* focus state can affect the stacking layer */
        client_calc_layer(client);
        frame_adjust_focus(client->frame);
	break;
    case EnterNotify:
        if (client_normal(client)) {
            if (ob_state == State_Starting) {
                /* move it to the top of the focus order */
                guint desktop = client->desktop;
                if (desktop == DESKTOP_ALL) desktop = screen_desktop;
                focus_order[desktop] = g_list_remove(focus_order[desktop],
                                                     client);
                focus_order[desktop] = g_list_prepend(focus_order[desktop],
                                                      client);
            } else if (config_focus_follow) {
#ifdef DEBUG_FOCUS
                g_message("EnterNotify on %lx, focusing window",
                          client->window);
#endif
                client_focus(client);
            }
        }
        break;
    case ConfigureRequest:
	/* compress these */
	while (XCheckTypedWindowEvent(ob_display, client->window,
				      ConfigureRequest, &ce)) {
            ++i;
	    /* XXX if this causes bad things.. we can compress config req's
	       with the same mask. */
	    e->xconfigurerequest.value_mask |=
		ce.xconfigurerequest.value_mask;
	    if (ce.xconfigurerequest.value_mask & CWX)
		e->xconfigurerequest.x = ce.xconfigurerequest.x;
	    if (ce.xconfigurerequest.value_mask & CWY)
		e->xconfigurerequest.y = ce.xconfigurerequest.y;
	    if (ce.xconfigurerequest.value_mask & CWWidth)
		e->xconfigurerequest.width = ce.xconfigurerequest.width;
	    if (ce.xconfigurerequest.value_mask & CWHeight)
		e->xconfigurerequest.height = ce.xconfigurerequest.height;
	    if (ce.xconfigurerequest.value_mask & CWBorderWidth)
		e->xconfigurerequest.border_width =
		    ce.xconfigurerequest.border_width;
	    if (ce.xconfigurerequest.value_mask & CWStackMode)
		e->xconfigurerequest.detail = ce.xconfigurerequest.detail;
	}

	/* if we are iconic (or shaded (fvwm does this)) ignore the event */
	if (client->iconic || client->shaded) return;

	if (e->xconfigurerequest.value_mask & CWBorderWidth)
	    client->border_width = e->xconfigurerequest.border_width;

	/* resize, then move, as specified in the EWMH section 7.7 */
	if (e->xconfigurerequest.value_mask & (CWWidth | CWHeight |
					       CWX | CWY)) {
	    int x, y, w, h;
	    Corner corner;
	       
	    x = (e->xconfigurerequest.value_mask & CWX) ?
		e->xconfigurerequest.x : client->area.x;
	    y = (e->xconfigurerequest.value_mask & CWY) ?
		e->xconfigurerequest.y : client->area.y;
	    w = (e->xconfigurerequest.value_mask & CWWidth) ?
		e->xconfigurerequest.width : client->area.width;
	    h = (e->xconfigurerequest.value_mask & CWHeight) ?
		e->xconfigurerequest.height : client->area.height;
	       
	    switch (client->gravity) {
	    case NorthEastGravity:
	    case EastGravity:
		corner = Corner_TopRight;
		break;
	    case SouthWestGravity:
	    case SouthGravity:
		corner = Corner_BottomLeft;
		break;
	    case SouthEastGravity:
		corner = Corner_BottomRight;
		break;
	    default:     /* NorthWest, Static, etc */
		corner = Corner_TopLeft;
	    }

	    client_configure(client, corner, x, y, w, h, FALSE, FALSE);
	}

	if (e->xconfigurerequest.value_mask & CWStackMode) {
	    switch (e->xconfigurerequest.detail) {
	    case Below:
	    case BottomIf:
		stacking_lower(client);
		break;

	    case Above:
	    case TopIf:
	    default:
		stacking_raise(client);
		break;
	    }
	}
	break;
    case UnmapNotify:
	if (client->ignore_unmaps) {
	    client->ignore_unmaps--;
	    break;
	}
	client_unmanage(client);
	break;
    case DestroyNotify:
	client_unmanage(client);
	break;
    case ReparentNotify:
	/* this is when the client is first taken captive in the frame */
	if (e->xreparent.parent == client->frame->plate) break;

	/*
	  This event is quite rare and is usually handled in unmapHandler.
	  However, if the window is unmapped when the reparent event occurs,
	  the window manager never sees it because an unmap event is not sent
	  to an already unmapped window.
	*/

	/* we don't want the reparent event, put it back on the stack for the
	   X server to deal with after we unmanage the window */
	XPutBackEvent(ob_display, e);
     
	client_unmanage(client);
	break;
    case MapRequest:
        g_message("MapRequest for 0x%lx", client->window);
        if (!client->iconic) break; /* this normally doesn't happen, but if it
                                       does, we don't want it! */
        if (screen_showing_desktop)
            screen_show_desktop(FALSE);
        client_iconify(client, FALSE, TRUE);
        if (!client->frame->visible)
            /* if its not visible still, then don't mess with it */
            break;
        if (client->shaded)
            client_shade(client, FALSE);
        client_focus(client);
        stacking_raise(client);
	break;
    case ClientMessage:
	/* validate cuz we query stuff off the client here */
	if (!client_validate(client)) break;
  
	if (e->xclient.format != 32) return;

	msgtype = e->xclient.message_type;
	if (msgtype == prop_atoms.wm_change_state) {
	    /* compress changes into a single change */
	    while (XCheckTypedWindowEvent(ob_display, e->type,
					  client->window, &ce)) {
		/* XXX: it would be nice to compress ALL messages of a
		   type, not just messages in a row without other
		   message types between. */
		if (ce.xclient.message_type != msgtype) {
		    XPutBackEvent(ob_display, &ce);
		    break;
		}
		e->xclient = ce.xclient;
	    }
	    client_set_wm_state(client, e->xclient.data.l[0]);
	} else if (msgtype == prop_atoms.net_wm_desktop) {
	    /* compress changes into a single change */
	    while (XCheckTypedWindowEvent(ob_display, e->type,
					  client->window, &ce)) {
		/* XXX: it would be nice to compress ALL messages of a
		   type, not just messages in a row without other
		   message types between. */
		if (ce.xclient.message_type != msgtype) {
		    XPutBackEvent(ob_display, &ce);
		    break;
		}
		e->xclient = ce.xclient;
	    }
            if ((unsigned)e->xclient.data.l[0] < screen_num_desktops ||
                (unsigned)e->xclient.data.l[0] == DESKTOP_ALL)
                client_set_desktop(client, (unsigned)e->xclient.data.l[0],
                                   FALSE);
	} else if (msgtype == prop_atoms.net_wm_state) {
	    /* can't compress these */
	    g_message("net_wm_state %s %ld %ld for 0x%lx",
		      (e->xclient.data.l[0] == 0 ? "Remove" :
		       e->xclient.data.l[0] == 1 ? "Add" :
		       e->xclient.data.l[0] == 2 ? "Toggle" : "INVALID"),
		      e->xclient.data.l[1], e->xclient.data.l[2],
		      client->window);
	    client_set_state(client, e->xclient.data.l[0],
			     e->xclient.data.l[1], e->xclient.data.l[2]);
	} else if (msgtype == prop_atoms.net_close_window) {
	    g_message("net_close_window for 0x%lx", client->window);
	    client_close(client);
	} else if (msgtype == prop_atoms.net_active_window) {
	    g_message("net_active_window for 0x%lx", client->window);
	    if (screen_showing_desktop)
		screen_show_desktop(FALSE);
	    if (client->iconic)
		client_iconify(client, FALSE, TRUE);
	    else if (!client->frame->visible)
		/* if its not visible for other reasons, then don't mess
		   with it */
		break;
            if (client->shaded)
                client_shade(client, FALSE);
            client_focus(client);
            stacking_raise(client);
	}
	break;
    case PropertyNotify:
	/* validate cuz we query stuff off the client here */
	if (!client_validate(client)) break;
  
	/* compress changes to a single property into a single change */
	while (XCheckTypedWindowEvent(ob_display, e->type,
				      client->window, &ce)) {
	    /* XXX: it would be nice to compress ALL changes to a property,
	       not just changes in a row without other props between. */
	    if (ce.xproperty.atom != e->xproperty.atom) {
		XPutBackEvent(ob_display, &ce);
		break;
	    }
	}

	msgtype = e->xproperty.atom;
	if (msgtype == XA_WM_NORMAL_HINTS) {
	    client_update_normal_hints(client);
	    /* normal hints can make a window non-resizable */
	    client_setup_decor_and_functions(client);
	}
	else if (msgtype == XA_WM_HINTS)
	    client_update_wmhints(client);
	else if (msgtype == XA_WM_TRANSIENT_FOR) {
	    client_update_transient_for(client);
	    client_get_type(client);
	    /* type may have changed, so update the layer */
	    client_calc_layer(client);
	    client_setup_decor_and_functions(client);
	}
	else if (msgtype == prop_atoms.net_wm_name ||
		 msgtype == prop_atoms.wm_name)
	    client_update_title(client);
	else if (msgtype == prop_atoms.net_wm_icon_name ||
		 msgtype == prop_atoms.wm_icon_name)
	    client_update_icon_title(client);
	else if (msgtype == prop_atoms.wm_class)
	    client_update_class(client);
	else if (msgtype == prop_atoms.wm_protocols) {
	    client_update_protocols(client);
	    client_setup_decor_and_functions(client);
	}
	else if (msgtype == prop_atoms.net_wm_strut)
	    client_update_strut(client);
	else if (msgtype == prop_atoms.net_wm_icon)
	    client_update_icons(client);
	else if (msgtype == prop_atoms.kwm_win_icon)
	    client_update_kwm_icon(client);
    default:
        ;
#ifdef SHAPE
        if (extensions_shape && e->type == extensions_shape_event_basep) {
            client->shaped = ((XShapeEvent*)e)->shaped;
            frame_adjust_shape(client->frame);
        }
#endif
    }
}
