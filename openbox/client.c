#include "client.h"
#include "dock.h"
#include "xerror.h"
#include "startup.h"
#include "screen.h"
#include "moveresize.h"
#include "prop.h"
#include "extensions.h"
#include "frame.h"
#include "event.h"
#include "grab.h"
#include "focus.h"
#include "stacking.h"
#include "dispatch.h"
#include "openbox.h"
#include "group.h"
#include "config.h"
#include "menu.h"
#include "render/render.h"

#include <glib.h>
#include <X11/Xutil.h>

/*! The event mask to grab on client windows */
#define CLIENT_EVENTMASK (PropertyChangeMask | FocusChangeMask | \
			  StructureNotifyMask)

#define CLIENT_NOPROPAGATEMASK (ButtonPressMask | ButtonReleaseMask | \
				ButtonMotionMask)

GList      *client_list      = NULL;

static void client_get_all(Client *self);
static void client_toggle_border(Client *self, gboolean show);
static void client_get_area(Client *self);
static void client_get_desktop(Client *self);
static void client_get_state(Client *self);
static void client_get_shaped(Client *self);
static void client_get_mwm_hints(Client *self);
static void client_get_gravity(Client *self);
static void client_showhide(Client *self);
static void client_change_allowed_actions(Client *self);
static void client_change_state(Client *self);
static void client_apply_startup_state(Client *self);

void client_startup()
{
    client_set_list();
}

void client_shutdown()
{
}

void client_set_list()
{
    Window *windows, *win_it;
    GList *it;
    guint size = g_list_length(client_list);

    /* create an array of the window ids */
    if (size > 0) {
	windows = g_new(Window, size);
	win_it = windows;
	for (it = client_list; it != NULL; it = it->next, ++win_it)
	    *win_it = ((Client*)it->data)->window;
    } else
	windows = NULL;

    PROP_SETA32(ob_root, net_client_list, window, (guint32*)windows, size);

    if (windows)
	g_free(windows);

    stacking_set_list();
}

/*
void client_foreach_transient(Client *self, ClientForeachFunc func, void *data)
{
    GSList *it;

    for (it = self->transients; it; it = it->next) {
        if (!func(it->data, data)) return;
        client_foreach_transient(it->data, func, data);
    }
}

void client_foreach_ancestor(Client *self, ClientForeachFunc func, void *data)
{
    if (self->transient_for) {
        if (self->transient_for != TRAN_GROUP) {
            if (!func(self->transient_for, data)) return;
            client_foreach_ancestor(self->transient_for, func, data);
        } else {
            GSList *it;

            for (it = self->group->members; it; it = it->next)
                if (it->data != self &&
                    !((Client*)it->data)->transient_for) {
                    if (!func(it->data, data)) return;
                    client_foreach_ancestor(it->data, func, data);
                }
        }
    }
}
*/

void client_manage_all()
{
    unsigned int i, j, nchild;
    Window w, *children;
    XWMHints *wmhints;
    XWindowAttributes attrib;

    XQueryTree(ob_display, ob_root, &w, &w, &children, &nchild);

    /* remove all icon windows from the list */
    for (i = 0; i < nchild; i++) {
	if (children[i] == None) continue;
	wmhints = XGetWMHints(ob_display, children[i]);
	if (wmhints) {
	    if ((wmhints->flags & IconWindowHint) &&
		(wmhints->icon_window != children[i]))
		for (j = 0; j < nchild; j++)
		    if (children[j] == wmhints->icon_window) {
			children[j] = None;
			break;
		    }
	    XFree(wmhints);
	}
    }

    for (i = 0; i < nchild; ++i) {
	if (children[i] == None)
	    continue;
	if (XGetWindowAttributes(ob_display, children[i], &attrib)) {
	    if (attrib.override_redirect) continue;

	    if (attrib.map_state != IsUnmapped)
		client_manage(children[i]);
	}
    }
    XFree(children);

    /* stack them as they were on startup!
       why with stacking_lower? Why, because then windows who aren't in the
       stacking list are on the top where you can see them instead of buried
       at the bottom! */
    for (i = startup_stack_size; i > 0; --i) {
        ObWindow *obw;

        w = startup_stack_order[i-1];
        obw = g_hash_table_lookup(window_map, &w);
        if (obw) {
            g_assert(WINDOW_IS_CLIENT(obw));
            stacking_lower(CLIENT_AS_WINDOW(obw));
        }
    }
    g_free(startup_stack_order);
    startup_stack_order = NULL;
    startup_stack_size = 0;

    if (config_focus_new) {
        ObWindow *active;

        active = g_hash_table_lookup(window_map, &startup_active);
        if (active) {
            g_assert(WINDOW_IS_CLIENT(active));
            if (!client_focus(WINDOW_AS_CLIENT(active)))
                focus_fallback(Fallback_NoFocus);
        } else
            focus_fallback(Fallback_NoFocus);
    }
}

void client_manage(Window window)
{
    Client *self;
    XEvent e;
    XWindowAttributes attrib;
    XSetWindowAttributes attrib_set;
    XWMHints *wmhint;
    gboolean activate = FALSE;

    grab_server(TRUE);

    /* check if it has already been unmapped by the time we started mapping
       the grab does a sync so we don't have to here */
    if (XCheckTypedWindowEvent(ob_display, window, DestroyNotify, &e) ||
	XCheckTypedWindowEvent(ob_display, window, UnmapNotify, &e)) {
	XPutBackEvent(ob_display, &e);

        grab_server(FALSE);
	return; /* don't manage it */
    }

    /* make sure it isn't an override-redirect window */
    if (!XGetWindowAttributes(ob_display, window, &attrib) ||
	attrib.override_redirect) {
        grab_server(FALSE);
	return; /* don't manage it */
    }
  
    /* is the window a docking app */
    if ((wmhint = XGetWMHints(ob_display, window))) {
	if ((wmhint->flags & StateHint) &&
	    wmhint->initial_state == WithdrawnState) {
            dock_add(window, wmhint);
            grab_server(FALSE);
	    XFree(wmhint);
	    return;
	}
	XFree(wmhint);
    }

    g_message("Managing window: %lx", window);

    /* choose the events we want to receive on the CLIENT window */
    attrib_set.event_mask = CLIENT_EVENTMASK;
    attrib_set.do_not_propagate_mask = CLIENT_NOPROPAGATEMASK;
    XChangeWindowAttributes(ob_display, window,
			    CWEventMask|CWDontPropagate, &attrib_set);


    /* create the Client struct, and populate it from the hints on the
       window */
    self = g_new(Client, 1);
    self->obwin.type = Window_Client;
    self->window = window;
    client_get_all(self);

    /* remove the client's border (and adjust re gravity) */
    client_toggle_border(self, FALSE);
     
    /* specify that if we exit, the window should not be destroyed and should
       be reparented back to root automatically */
    XChangeSaveSet(ob_display, window, SetModeInsert);

    /* create the decoration frame for the client window */
    self->frame = frame_new();

    frame_grab_client(self->frame, self);

    client_apply_startup_state(self);

    grab_server(FALSE);

    /* add to client list/map */
    client_list = g_list_append(client_list, self);
    g_hash_table_insert(window_map, &self->window, self);

    /* update the focus lists */
    focus_order_add_new(self);

    /* focus the new window? */
    if (ob_state != State_Starting && config_focus_new &&
        (self->type == Type_Normal || self->type == Type_Dialog)) {
        gboolean group_foc = FALSE;
        
        if (self->group) {
            GSList *it;

            for (it = self->group->members; it; it = it->next) {
                if (client_focused(it->data)) {
                    group_foc = TRUE;
                    break;
                }
            }
        }
        /* note the check against Type_Normal/Dialog, not client_normal(self),
           which would also include other types. in this case we want more
           strict rules for focus */
        if ((group_foc ||
             (!self->transient_for && (!self->group ||
                                       !self->group->members->next))) ||
            client_search_focus_tree_full(self) ||
            !focus_client ||
            !client_normal(focus_client)) {
            /* activate the window */
            stacking_add(CLIENT_AS_WINDOW(self));
            activate = TRUE;
        } else {
            /* try to not get in the way */
            stacking_add_nonintrusive(CLIENT_AS_WINDOW(self));
        }
    } else {
        stacking_add(CLIENT_AS_WINDOW(self));
    }

    screen_update_struts();

    /* make sure the window is visible */
    client_move_onscreen(self);

    dispatch_client(Event_Client_New, self, 0, 0);

    client_showhide(self);

    if (activate) client_activate(self);

    /* update the list hints */
    client_set_list();

    dispatch_client(Event_Client_Mapped, self, 0, 0);

    g_message("Managed window 0x%lx (%s)", window, self->class);
}

void client_unmanage_all()
{
    while (client_list != NULL)
	client_unmanage(client_list->data);
}

/* called by client_unmanage() to close any menus referencing this client */
void client_close_menus(gpointer key, gpointer value, gpointer self)
{
    if (((Menu *)value)->client == (Client *)self)
        menu_hide((Menu *)value);
}

void client_unmanage(Client *self)
{
    int j;
    GSList *it;

    g_message("Unmanaging window: %lx (%s)", self->window, self->class);

    dispatch_client(Event_Client_Destroy, self, 0, 0);
    g_assert(self != NULL);

    /* remove the window from our save set */
    XChangeSaveSet(ob_display, self->window, SetModeDelete);

    /* we dont want events no more */
    XSelectInput(ob_display, self->window, NoEventMask);

    frame_hide(self->frame);

    client_list = g_list_remove(client_list, self);
    stacking_remove(self);
    g_hash_table_remove(window_map, &self->window);

    /* update the focus lists */
    focus_order_remove(self);

    /* once the client is out of the list, update the struts to remove it's
       influence */
    screen_update_struts();

    /* tell our parent(s) that we're gone */
    if (self->transient_for == TRAN_GROUP) { /* transient of group */
        GSList *it;

        for (it = self->group->members; it; it = it->next)
            if (it->data != self)
                ((Client*)it->data)->transients =
                    g_slist_remove(((Client*)it->data)->transients, self);
    } else if (self->transient_for) {        /* transient of window */
	self->transient_for->transients =
	    g_slist_remove(self->transient_for->transients, self);
    }

    /* tell our transients that we're gone */
    for (it = self->transients; it != NULL; it = it->next) {
        if (((Client*)it->data)->transient_for != TRAN_GROUP) {
            ((Client*)it->data)->transient_for = NULL;
            client_calc_layer(it->data);
        }
    }

    if (moveresize_client == self)
        moveresize_end(TRUE);

    /* close any windows that are attached to this window */
    g_hash_table_foreach(menu_hash, client_close_menus, self);

    
    if (focus_client == self) {
        XEvent e;

        /* focus the last focused window on the desktop, and ignore enter
           events from the unmap so it doesnt mess with the focus */
        while (XCheckTypedEvent(ob_display, EnterNotify, &e));
        client_unfocus(self);
    }

    /* remove from its group */
    if (self->group) {
        group_remove(self->group, self);
        self->group = NULL;
    }

    /* dispatch the unmapped event */
    dispatch_client(Event_Client_Unmapped, self, 0, 0);
    g_assert(self != NULL);

    /* give the client its border back */
    client_toggle_border(self, TRUE);

    /* reparent the window out of the frame, and free the frame */
    frame_release_client(self->frame, self);
    self->frame = NULL;
     
    if (ob_state != State_Exiting) {
	/* these values should not be persisted across a window
	   unmapping/mapping */
	prop_erase(self->window, prop_atoms.net_wm_desktop);
	prop_erase(self->window, prop_atoms.net_wm_state);
    } else {
	/* if we're left in an iconic state, the client wont be mapped. this is
	   bad, since we will no longer be managing the window on restart */
	if (self->iconic)
	    XMapWindow(ob_display, self->window);
    }


    g_message("Unmanaged window 0x%lx", self->window);

    /* free all data allocated in the client struct */
    g_slist_free(self->transients);
    for (j = 0; j < self->nicons; ++j)
	g_free(self->icons[j].data);
    if (self->nicons > 0)
	g_free(self->icons);
    g_free(self->title);
    g_free(self->icon_title);
    g_free(self->name);
    g_free(self->class);
    g_free(self->role);
    g_free(self);
     
    /* update the list hints */
    client_set_list();
}

void client_move_onscreen(Client *self)
{
    Rect *a;
    int x = self->frame->area.x, y = self->frame->area.y;

    a = screen_area(self->desktop);
    if (x >= a->x + a->width - 1)
        x = a->x + a->width - self->frame->area.width;
    if (y >= a->y + a->height - 1)
        y = a->y + a->height - self->frame->area.height;
    if (x + self->frame->area.width - 1 < a->x)
        x = a->x;
    if (y + self->frame->area.height - 1< a->y)
        y = a->y;

    frame_frame_gravity(self->frame, &x, &y); /* get where the client
                                                 should be */
    client_configure(self , Corner_TopLeft, x, y,
                     self->area.width, self->area.height,
                     TRUE, TRUE);
}

static void client_toggle_border(Client *self, gboolean show)
{
    /* adjust our idea of where the client is, based on its border. When the
       border is removed, the client should now be considered to be in a
       different position.
       when re-adding the border to the client, the same operation needs to be
       reversed. */
    int oldx = self->area.x, oldy = self->area.y;
    int x = oldx, y = oldy;
    switch(self->gravity) {
    default:
    case NorthWestGravity:
    case WestGravity:
    case SouthWestGravity:
	break;
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
	if (show) x -= self->border_width * 2;
	else      x += self->border_width * 2;
	break;
    case NorthGravity:
    case SouthGravity:
    case CenterGravity:
    case ForgetGravity:
    case StaticGravity:
	if (show) x -= self->border_width;
	else      x += self->border_width;
	break;
    }
    switch(self->gravity) {
    default:
    case NorthWestGravity:
    case NorthGravity:
    case NorthEastGravity:
	break;
    case SouthWestGravity:
    case SouthGravity:
    case SouthEastGravity:
	if (show) y -= self->border_width * 2;
	else      y += self->border_width * 2;
	break;
    case WestGravity:
    case EastGravity:
    case CenterGravity:
    case ForgetGravity:
    case StaticGravity:
	if (show) y -= self->border_width;
	else      y += self->border_width;
	break;
    }
    self->area.x = x;
    self->area.y = y;

    if (show) {
	XSetWindowBorderWidth(ob_display, self->window, self->border_width);

	/* move the client so it is back it the right spot _with_ its
	   border! */
	if (x != oldx || y != oldy)
	    XMoveWindow(ob_display, self->window, x, y);
    } else
	XSetWindowBorderWidth(ob_display, self->window, 0);
}


static void client_get_all(Client *self)
{
    /* update EVERYTHING!! */

    self->ignore_unmaps = 0;
  
    /* defaults */
    self->frame = NULL;
    self->title = self->icon_title = NULL;
    self->title_count = 1;
    self->name = self->class = self->role = NULL;
    self->wmstate = NormalState;
    self->transient = FALSE;
    self->transients = NULL;
    self->transient_for = NULL;
    self->layer = -1;
    self->urgent = FALSE;
    self->positioned = FALSE;
    self->disabled_decorations = 0;
    self->group = NULL;
    self->nicons = 0;

    client_get_area(self);
    client_update_transient_for(self);
    client_update_wmhints(self);
    client_get_desktop(self);
    client_get_state(self);
    client_get_shaped(self);

    client_get_mwm_hints(self);
    client_get_type(self);/* this can change the mwmhints for special cases */

    client_update_protocols(self);

    client_get_gravity(self); /* get the attribute gravity */
    client_update_normal_hints(self); /* this may override the attribute
					 gravity */

    /* got the type, the mwmhints, the protocols, and the normal hints
       (min/max sizes), so we're ready to set up the decorations/functions */
    client_setup_decor_and_functions(self);
  
    client_update_title(self);
    client_update_class(self);
    client_update_strut(self);
    client_update_icons(self);

    client_change_state(self);
}

static void client_get_area(Client *self)
{
    XWindowAttributes wattrib;
    Status ret;
  
    ret = XGetWindowAttributes(ob_display, self->window, &wattrib);
    g_assert(ret != BadWindow);

    RECT_SET(self->area, wattrib.x, wattrib.y, wattrib.width, wattrib.height);
    self->border_width = wattrib.border_width;
}

static void client_get_desktop(Client *self)
{
    guint32 d;

    if (PROP_GET32(self->window, net_wm_desktop, cardinal, &d)) {
	if (d >= screen_num_desktops && d != DESKTOP_ALL)
	    d = screen_num_desktops - 1;
	self->desktop = d;
    } else { 
        gboolean trdesk = FALSE;

       if (self->transient_for) {
           if (self->transient_for != TRAN_GROUP) {
                self->desktop = self->transient_for->desktop;
                trdesk = TRUE;
            } else {
                GSList *it;

                for (it = self->group->members; it; it = it->next)
                    if (it->data != self &&
                        !((Client*)it->data)->transient_for) {
                        self->desktop = ((Client*)it->data)->desktop;
                        trdesk = TRUE;
                        break;
                    }
            }
       }
       if (!trdesk)
           /* defaults to the current desktop */
           self->desktop = screen_desktop;

        /* set the desktop hint, to make sure that it always exists */
        PROP_SET32(self->window, net_wm_desktop, cardinal, self->desktop);
    }
}

static void client_get_state(Client *self)
{
    guint32 *state;
    guint num;
  
    self->modal = self->shaded = self->max_horz = self->max_vert =
	self->fullscreen = self->above = self->below = self->iconic =
	self->skip_taskbar = self->skip_pager = FALSE;

    if (PROP_GETA32(self->window, net_wm_state, atom, &state, &num)) {
	gulong i;
	for (i = 0; i < num; ++i) {
	    if (state[i] == prop_atoms.net_wm_state_modal)
		self->modal = TRUE;
	    else if (state[i] == prop_atoms.net_wm_state_shaded)
		self->shaded = TRUE;
	    else if (state[i] == prop_atoms.net_wm_state_hidden)
		self->iconic = TRUE;
	    else if (state[i] == prop_atoms.net_wm_state_skip_taskbar)
		self->skip_taskbar = TRUE;
	    else if (state[i] == prop_atoms.net_wm_state_skip_pager)
		self->skip_pager = TRUE;
	    else if (state[i] == prop_atoms.net_wm_state_fullscreen)
		self->fullscreen = TRUE;
	    else if (state[i] == prop_atoms.net_wm_state_maximized_vert)
		self->max_vert = TRUE;
	    else if (state[i] == prop_atoms.net_wm_state_maximized_horz)
		self->max_horz = TRUE;
	    else if (state[i] == prop_atoms.net_wm_state_above)
		self->above = TRUE;
	    else if (state[i] == prop_atoms.net_wm_state_below)
		self->below = TRUE;
	}

	g_free(state);
    }
}

static void client_get_shaped(Client *self)
{
    self->shaped = FALSE;
#ifdef   SHAPE
    if (extensions_shape) {
	int foo;
	guint ufoo;
	int s;

	XShapeSelectInput(ob_display, self->window, ShapeNotifyMask);

	XShapeQueryExtents(ob_display, self->window, &s, &foo,
			   &foo, &ufoo, &ufoo, &foo, &foo, &foo, &ufoo,
			   &ufoo);
	self->shaped = (s != 0);
    }
#endif
}

void client_update_transient_for(Client *self)
{
    Window t = None;
    Client *c = NULL;

    if (XGetTransientForHint(ob_display, self->window, &t)) {
	self->transient = TRUE;
        if (t != self->window) { /* cant be transient to itself! */
            c = g_hash_table_lookup(window_map, &t);
            /* if this happens then we need to check for it*/
            g_assert(c != self);
            g_assert(!c || WINDOW_IS_CLIENT(c));
            
            if (!c && self->group) {
                /* not transient to a client, see if it is transient for a
                   group */
                if (t == self->group->leader ||
                    t == None ||
                    t == ob_root) {
                    /* window is a transient for its group! */
                    c = TRAN_GROUP;
                }
            }
        }
    } else
	self->transient = FALSE;

    /* if anything has changed... */
    if (c != self->transient_for) {
	if (self->transient_for == TRAN_GROUP) { /* transient of group */
            GSList *it;

	    /* remove from old parents */
            for (it = self->group->members; it; it = it->next)
                if (it->data != self &&
                    !((Client*)it->data)->transient_for)
                    ((Client*)it->data)->transients =
                        g_slist_remove(((Client*)it->data)->transients, self);
        } else if (self->transient_for != NULL) { /* transient of window */
	    /* remove from old parent */
	    self->transient_for->transients =
                g_slist_remove(self->transient_for->transients, self);
        }
	self->transient_for = c;
	if (self->transient_for == TRAN_GROUP) { /* transient of group */
            GSList *it;

	    /* add to new parents */
            for (it = self->group->members; it; it = it->next)
                if (it->data != self &&
                    !((Client*)it->data)->transient_for)
                    ((Client*)it->data)->transients =
                        g_slist_append(((Client*)it->data)->transients, self);

            /* remove all transients which are in the group, that causes
               circlular pointer hell of doom */
            for (it = self->group->members; it; it = it->next) {
                GSList *sit, *next;
                for (sit = self->transients; sit; sit = next) {
                    next = sit->next;
                    if (sit->data == it->data)
                        self->transients = g_slist_remove(self->transients,
                                                          sit->data);
                }
            }
        } else if (self->transient_for != NULL) { /* transient of window */
	    /* add to new parent */
	    self->transient_for->transients =
                g_slist_append(self->transient_for->transients, self);
        }
    }
}

static void client_get_mwm_hints(Client *self)
{
    guint num;
    guint32 *hints;

    self->mwmhints.flags = 0; /* default to none */

    if (PROP_GETA32(self->window, motif_wm_hints, motif_wm_hints,
                    &hints, &num)) {
	if (num >= MWM_ELEMENTS) {
	    self->mwmhints.flags = hints[0];
	    self->mwmhints.functions = hints[1];
	    self->mwmhints.decorations = hints[2];
	}
	g_free(hints);
    }
}

void client_get_type(Client *self)
{
    guint num, i;
    guint32 *val;

    self->type = -1;
  
    if (PROP_GETA32(self->window, net_wm_window_type, atom, &val, &num)) {
	/* use the first value that we know about in the array */
	for (i = 0; i < num; ++i) {
	    if (val[i] == prop_atoms.net_wm_window_type_desktop)
		self->type = Type_Desktop;
	    else if (val[i] == prop_atoms.net_wm_window_type_dock)
		self->type = Type_Dock;
	    else if (val[i] == prop_atoms.net_wm_window_type_toolbar)
		self->type = Type_Toolbar;
	    else if (val[i] == prop_atoms.net_wm_window_type_menu)
		self->type = Type_Menu;
	    else if (val[i] == prop_atoms.net_wm_window_type_utility)
		self->type = Type_Utility;
	    else if (val[i] == prop_atoms.net_wm_window_type_splash)
		self->type = Type_Splash;
	    else if (val[i] == prop_atoms.net_wm_window_type_dialog)
		self->type = Type_Dialog;
	    else if (val[i] == prop_atoms.net_wm_window_type_normal)
		self->type = Type_Normal;
	    else if (val[i] == prop_atoms.kde_net_wm_window_type_override) {
		/* prevent this window from getting any decor or
		   functionality */
		self->mwmhints.flags &= (MwmFlag_Functions |
					 MwmFlag_Decorations);
		self->mwmhints.decorations = 0;
		self->mwmhints.functions = 0;
	    }
	    if (self->type != (WindowType) -1)
		break; /* grab the first legit type */
	}
	g_free(val);
    }
    
    if (self->type == (WindowType) -1) {
	/*the window type hint was not set, which means we either classify
	  ourself as a normal window or a dialog, depending on if we are a
	  transient. */
	if (self->transient)
	    self->type = Type_Dialog;
	else
	    self->type = Type_Normal;
    }
}

void client_update_protocols(Client *self)
{
    guint32 *proto;
    guint num_return, i;

    self->focus_notify = FALSE;
    self->delete_window = FALSE;

    if (PROP_GETA32(self->window, wm_protocols, atom, &proto, &num_return)) {
	for (i = 0; i < num_return; ++i) {
	    if (proto[i] == prop_atoms.wm_delete_window) {
		/* this means we can request the window to close */
		self->delete_window = TRUE;
	    } else if (proto[i] == prop_atoms.wm_take_focus)
		/* if this protocol is requested, then the window will be
		   notified whenever we want it to receive focus */
		self->focus_notify = TRUE;
	}
	g_free(proto);
    }
}

static void client_get_gravity(Client *self)
{
    XWindowAttributes wattrib;
    Status ret;

    ret = XGetWindowAttributes(ob_display, self->window, &wattrib);
    g_assert(ret != BadWindow);
    self->gravity = wattrib.win_gravity;
}

void client_update_normal_hints(Client *self)
{
    XSizeHints size;
    long ret;
    int oldgravity = self->gravity;

    /* defaults */
    self->min_ratio = 0.0f;
    self->max_ratio = 0.0f;
    SIZE_SET(self->size_inc, 1, 1);
    SIZE_SET(self->base_size, 0, 0);
    SIZE_SET(self->min_size, 0, 0);
    SIZE_SET(self->max_size, G_MAXINT, G_MAXINT);

    /* get the hints from the window */
    if (XGetWMNormalHints(ob_display, self->window, &size, &ret)) {
	self->positioned = !!(size.flags & (PPosition|USPosition));

	if (size.flags & PWinGravity) {
	    self->gravity = size.win_gravity;
      
	    /* if the client has a frame, i.e. has already been mapped and
	       is changing its gravity */
	    if (self->frame && self->gravity != oldgravity) {
		/* move our idea of the client's position based on its new
		   gravity */
		self->area.x = self->frame->area.x;
		self->area.y = self->frame->area.y;
		frame_frame_gravity(self->frame, &self->area.x, &self->area.y);
	    }
	}

	if (size.flags & PAspect) {
	    if (size.min_aspect.y)
		self->min_ratio = (float)size.min_aspect.x / size.min_aspect.y;
	    if (size.max_aspect.y)
		self->max_ratio = (float)size.max_aspect.x / size.max_aspect.y;
	}

	if (size.flags & PMinSize)
	    SIZE_SET(self->min_size, size.min_width, size.min_height);
    
	if (size.flags & PMaxSize)
	    SIZE_SET(self->max_size, size.max_width, size.max_height);
    
	if (size.flags & PBaseSize)
	    SIZE_SET(self->base_size, size.base_width, size.base_height);
    
	if (size.flags & PResizeInc)
	    SIZE_SET(self->size_inc, size.width_inc, size.height_inc);
    }
}

void client_setup_decor_and_functions(Client *self)
{
    /* start with everything (cept fullscreen) */
    self->decorations = Decor_Titlebar | Decor_Handle | Decor_Border |
	Decor_Icon | Decor_AllDesktops | Decor_Iconify | Decor_Maximize |
        Decor_Shade;
    self->functions = Func_Resize | Func_Move | Func_Iconify | Func_Maximize |
	Func_Shade;
    if (self->delete_window) {
	self->decorations |= Decor_Close;
	self->functions |= Func_Close;
    }

    if (!(self->min_size.width < self->max_size.width ||
	  self->min_size.height < self->max_size.height)) {
	self->decorations &= ~(Decor_Maximize | Decor_Handle);
	self->functions &= ~(Func_Resize | Func_Maximize);
    }

    switch (self->type) {
    case Type_Normal:
	/* normal windows retain all of the possible decorations and
	   functionality, and are the only windows that you can fullscreen */
	self->functions |= Func_Fullscreen;
	break;

    case Type_Dialog:
    case Type_Utility:
	/* these windows cannot be maximized */
	self->decorations &= ~Decor_Maximize;
	self->functions &= ~Func_Maximize;
	break;

    case Type_Menu:
    case Type_Toolbar:
	/* these windows get less functionality */
	self->decorations &= ~(Decor_Iconify | Decor_Handle);
	self->functions &= ~(Func_Iconify | Func_Resize);
	break;

    case Type_Desktop:
    case Type_Dock:
    case Type_Splash:
	/* none of these windows are manipulated by the window manager */
	self->decorations = 0;
	self->functions = 0;
	break;
    }

    /* Mwm Hints are applied subtractively to what has already been chosen for
       decor and functionality */
    if (self->mwmhints.flags & MwmFlag_Decorations) {
	if (! (self->mwmhints.decorations & MwmDecor_All)) {
	    if (! (self->mwmhints.decorations & MwmDecor_Border))
		self->decorations &= ~Decor_Border;
	    if (! (self->mwmhints.decorations & MwmDecor_Handle))
		self->decorations &= ~Decor_Handle;
	    if (! (self->mwmhints.decorations & MwmDecor_Title))
		self->decorations &= ~Decor_Titlebar;
	    if (! (self->mwmhints.decorations & MwmDecor_Iconify))
		self->decorations &= ~Decor_Iconify;
	    if (! (self->mwmhints.decorations & MwmDecor_Maximize))
		self->decorations &= ~Decor_Maximize;
	}
    }

    if (self->mwmhints.flags & MwmFlag_Functions) {
	if (! (self->mwmhints.functions & MwmFunc_All)) {
	    if (! (self->mwmhints.functions & MwmFunc_Resize))
		self->functions &= ~Func_Resize;
	    if (! (self->mwmhints.functions & MwmFunc_Move))
		self->functions &= ~Func_Move;
	    if (! (self->mwmhints.functions & MwmFunc_Iconify))
		self->functions &= ~Func_Iconify;
	    if (! (self->mwmhints.functions & MwmFunc_Maximize))
		self->functions &= ~Func_Maximize;
	    /* dont let mwm hints kill the close button
	       if (! (self->mwmhints.functions & MwmFunc_Close))
	       self->functions &= ~Func_Close; */
	}
    }

    /* can't maximize without moving/resizing */
    if (!((self->functions & Func_Move) && (self->functions & Func_Resize)))
	self->functions &= ~(Func_Maximize | Func_Fullscreen);

    /* finally, user specified disabled decorations are applied to subtract
       decorations */
    if (self->disabled_decorations & Decor_Titlebar)
	self->decorations &= ~Decor_Titlebar;
    if (self->disabled_decorations & Decor_Handle)
	self->decorations &= ~Decor_Handle;
    if (self->disabled_decorations & Decor_Border)
	self->decorations &= ~Decor_Border;
    if (self->disabled_decorations & Decor_Iconify)
	self->decorations &= ~Decor_Iconify;
    if (self->disabled_decorations & Decor_Maximize)
	self->decorations &= ~Decor_Maximize;
    if (self->disabled_decorations & Decor_AllDesktops)
	self->decorations &= ~Decor_AllDesktops;
    if (self->disabled_decorations & Decor_Shade)
	self->decorations &= ~Decor_Shade;
    if (self->disabled_decorations & Decor_Close)
	self->decorations &= ~Decor_Close;

    /* if we don't have a titlebar, then we cannot shade! */
    if (!(self->decorations & Decor_Titlebar))
	self->functions &= ~Func_Shade;

    /* now we need to check against rules for the client's current state */
    if (self->fullscreen) {
	self->functions &= (Func_Close | Func_Fullscreen | Func_Iconify);
	self->decorations = 0;
    }

    client_change_allowed_actions(self);

    if (self->frame) {
        /* this makes sure that these windows appear on all desktops */
        if (self->type == Type_Desktop && self->desktop != DESKTOP_ALL)
            client_set_desktop(self, DESKTOP_ALL, FALSE);

	/* change the decors on the frame, and with more/less decorations,
           we may also need to be repositioned */
	frame_adjust_area(self->frame, TRUE, TRUE);
	/* with new decor, the window's maximized size may change */
	client_reconfigure(self);
    } else {
        /* this makes sure that these windows appear on all desktops */
        if (self->type == Type_Desktop && self->desktop != DESKTOP_ALL)
            self->desktop = DESKTOP_ALL;
    }
}

static void client_change_allowed_actions(Client *self)
{
    guint32 actions[9];
    int num = 0;

    /* desktop windows are kept on all desktops */
    if (self->type != Type_Desktop)
        actions[num++] = prop_atoms.net_wm_action_change_desktop;

    if (self->functions & Func_Shade)
	actions[num++] = prop_atoms.net_wm_action_shade;
    if (self->functions & Func_Close)
	actions[num++] = prop_atoms.net_wm_action_close;
    if (self->functions & Func_Move)
	actions[num++] = prop_atoms.net_wm_action_move;
    if (self->functions & Func_Iconify)
	actions[num++] = prop_atoms.net_wm_action_minimize;
    if (self->functions & Func_Resize)
	actions[num++] = prop_atoms.net_wm_action_resize;
    if (self->functions & Func_Fullscreen)
	actions[num++] = prop_atoms.net_wm_action_fullscreen;
    if (self->functions & Func_Maximize) {
	actions[num++] = prop_atoms.net_wm_action_maximize_horz;
	actions[num++] = prop_atoms.net_wm_action_maximize_vert;
    }

    PROP_SETA32(self->window, net_wm_allowed_actions, atom, actions, num);

    /* make sure the window isn't breaking any rules now */

    if (!(self->functions & Func_Shade) && self->shaded) {
	if (self->frame) client_shade(self, FALSE);
	else self->shaded = FALSE;
    }
    if (!(self->functions & Func_Iconify) && self->iconic) {
        g_message("UNSETTING ICONIC");
	if (self->frame) client_iconify(self, FALSE, TRUE);
	else self->iconic = FALSE;
    }
    if (!(self->functions & Func_Fullscreen) && self->fullscreen) {
	if (self->frame) client_fullscreen(self, FALSE, TRUE);
	else self->fullscreen = FALSE;
    }
    if (!(self->functions & Func_Maximize) && (self->max_horz ||
					       self->max_vert)) {
	if (self->frame) client_maximize(self, FALSE, 0, TRUE);
	else self->max_vert = self->max_horz = FALSE;
    }
}

void client_reconfigure(Client *self)
{
    client_configure(self, Corner_TopLeft, self->area.x, self->area.y,
                     self->area.width, self->area.height, TRUE, TRUE);
}

void client_update_wmhints(Client *self)
{
    XWMHints *hints;
    gboolean ur = FALSE;
    GSList *it;

    /* assume a window takes input if it doesnt specify */
    self->can_focus = TRUE;
  
    if ((hints = XGetWMHints(ob_display, self->window)) != NULL) {
	if (hints->flags & InputHint)
	    self->can_focus = hints->input;

	/* only do this when first managing the window *AND* when we aren't
           starting up! */
	if (ob_state != State_Starting && self->frame == NULL)
            if (hints->flags & StateHint)
                self->iconic = hints->initial_state == IconicState;

	if (hints->flags & XUrgencyHint)
	    ur = TRUE;

	if (!(hints->flags & WindowGroupHint))
            hints->window_group = None;

        /* did the group state change? */
        if (hints->window_group !=
            (self->group ? self->group->leader : None)) {
            /* remove from the old group if there was one */
            if (self->group != NULL) {
                /* remove transients of the group */
                for (it = self->group->members; it; it = it->next)
                    self->transients = g_slist_remove(self->transients,
                                                      it->data);
                group_remove(self->group, self);
                self->group = NULL;
            }
            if (hints->window_group != None) {
                self->group = group_add(hints->window_group, self);

                /* i can only have transients from the group if i am not
                   transient myself */
                if (!self->transient_for) {
                    /* add other transients of the group that are already
                       set up */
                    for (it = self->group->members; it; it = it->next)
                        if (it->data != self &&
                            ((Client*)it->data)->transient_for == TRAN_GROUP)
                            self->transients = g_slist_append(self->transients,
                                                              it->data);
                }
            }

            /* the WM_HINTS can contain an icon */
            client_update_icons(self);

            /* because the self->transient flag wont change from this call,
               we don't need to update the window's type and such, only its
               transient_for, and the transients lists of other windows in
               the group may be affected */
            client_update_transient_for(self);
        }

	XFree(hints);
    }

    if (ur != self->urgent) {
	self->urgent = ur;
	g_message("Urgent Hint for 0x%lx: %s", self->window,
		  ur ? "ON" : "OFF");
	/* fire the urgent callback if we're mapped, otherwise, wait until
	   after we're mapped */
	if (self->frame)
            dispatch_client(Event_Client_Urgent, self, self->urgent, 0);
    }
}

void client_update_title(Client *self)
{
    GList *it;
    guint32 nums;
    guint i;
    char *data = NULL;
    gboolean read_title;

    g_free(self->title);
     
    /* try netwm */
    if (!PROP_GETS(self->window, net_wm_name, utf8, &data))
	/* try old x stuff */
	if (!PROP_GETS(self->window, wm_name, locale, &data))
	    data = g_strdup("Unnamed Window");

    /* look for duplicates and append a number */
    nums = 0;
    for (it = client_list; it; it = it->next)
        if (it->data != self) {
            Client *c = it->data;
            if (0 == strncmp(c->title, data, strlen(data)))
                nums |= 1 << c->title_count;
        }
    /* find first free number */
    for (i = 1; i <= 32; ++i)
        if (!(nums & (1 << i))) {
            if (self->title_count == 1 || i == 1)
                self->title_count = i;
            break;
        }
    /* dont display the number for the first window */
    if (self->title_count > 1) {
        char *vdata, *ndata;
        ndata = g_strdup_printf(" - [%u]", self->title_count);
        vdata = g_strconcat(data, ndata, NULL);
        g_free(ndata);
        g_free(data);
        data = vdata;
    }

    PROP_SETS(self->window, net_wm_visible_name, data);

    self->title = data;

    if (self->frame)
	frame_adjust_title(self->frame);

    /* update the icon title */
    data = NULL;
    g_free(self->icon_title);

    read_title = TRUE;
    /* try netwm */
    if (!PROP_GETS(self->window, net_wm_icon_name, utf8, &data))
	/* try old x stuff */
	if (!PROP_GETS(self->window, wm_icon_name, locale, &data)) {
            data = g_strdup(self->title);
            read_title = FALSE;
        }

    /* append the title count, dont display the number for the first window */
    if (read_title && self->title_count > 1) {
        char *vdata, *ndata;
        ndata = g_strdup_printf(" - [%u]", self->title_count);
        vdata = g_strconcat(data, ndata, NULL);
        g_free(ndata);
        g_free(data);
        data = vdata;
    }

    PROP_SETS(self->window, net_wm_visible_icon_name, data);

    self->icon_title = data;
}

void client_update_class(Client *self)
{
    char **data;
    char *s;

    if (self->name) g_free(self->name);
    if (self->class) g_free(self->class);
    if (self->role) g_free(self->role);

    self->name = self->class = self->role = NULL;

    if (PROP_GETSS(self->window, wm_class, locale, &data)) {
        if (data[0]) {
	    self->name = g_strdup(data[0]);
            if (data[1])
                self->class = g_strdup(data[1]);
        }
        g_strfreev(data);     
    }

    if (PROP_GETS(self->window, wm_window_role, locale, &s))
	self->role = g_strdup(s);

    if (self->name == NULL) self->name = g_strdup("");
    if (self->class == NULL) self->class = g_strdup("");
    if (self->role == NULL) self->role = g_strdup("");
}

void client_update_strut(Client *self)
{
    guint num;
    guint32 *data;

    if (!PROP_GETA32(self->window, net_wm_strut, cardinal, &data, &num)) {
	STRUT_SET(self->strut, 0, 0, 0, 0);
    } else {
        if (num == 4)
            STRUT_SET(self->strut, data[0], data[2], data[1], data[3]);
        else
            STRUT_SET(self->strut, 0, 0, 0, 0);
	g_free(data);
    }

    /* updating here is pointless while we're being mapped cuz we're not in
       the client list yet */
    if (self->frame)
	screen_update_struts();
}

void client_update_icons(Client *self)
{
    guint num;
    guint32 *data;
    guint w, h, i;
    int j;

    for (j = 0; j < self->nicons; ++j)
	g_free(self->icons[j].data);
    if (self->nicons > 0)
	g_free(self->icons);
    self->nicons = 0;

    if (PROP_GETA32(self->window, net_wm_icon, cardinal, &data, &num)) {
	/* figure out how many valid icons are in here */
	i = 0;
	while (num - i > 2) {
	    w = data[i++];
	    h = data[i++];
	    i += w * h;
	    if (i > num || w*h == 0) break;
	    ++self->nicons;
	}

	self->icons = g_new(Icon, self->nicons);
    
	/* store the icons */
	i = 0;
	for (j = 0; j < self->nicons; ++j) {
            guint x, y, t;

	    w = self->icons[j].width = data[i++];
	    h = self->icons[j].height = data[i++];

            if (w*h == 0) continue;

	    self->icons[j].data = g_new(pixel32, w * h);
            for (x = 0, y = 0, t = 0; t < w * h; ++t, ++x, ++i) {
                if (x >= w) {
                    x = 0;
                    ++y;
                }
                self->icons[j].data[t] =
                    (((data[i] >> 24) & 0xff) << default_alpha_offset) +
                    (((data[i] >> 16) & 0xff) << default_red_offset) +
                    (((data[i] >> 8) & 0xff) << default_green_offset) +
                    (((data[i] >> 0) & 0xff) << default_blue_offset);
            }
	    g_assert(i <= num);
	}

	g_free(data);
    } else if (PROP_GETA32(self->window, kwm_win_icon,
                           kwm_win_icon, &data, &num)) {
        if (num == 2) {
            self->nicons++;
            self->icons = g_new(Icon, self->nicons);
            xerror_set_ignore(TRUE);
            if (!render_pixmap_to_rgba(data[0], data[1],
                                       &self->icons[self->nicons-1].width,
                                       &self->icons[self->nicons-1].height,
                                       &self->icons[self->nicons-1].data)) {
                g_free(&self->icons[self->nicons-1]);
                self->nicons--;
            }
            xerror_set_ignore(FALSE);
        }
        g_free(data);
    } else {
        XWMHints *hints;

        if ((hints = XGetWMHints(ob_display, self->window))) {
            if (hints->flags & IconPixmapHint) {
                self->nicons++;
                self->icons = g_new(Icon, self->nicons);
                xerror_set_ignore(TRUE);
                if (!render_pixmap_to_rgba(hints->icon_pixmap,
                                           (hints->flags & IconMaskHint ?
                                            hints->icon_mask : None),
                                           &self->icons[self->nicons-1].width,
                                           &self->icons[self->nicons-1].height,
                                           &self->icons[self->nicons-1].data)){
                    g_free(&self->icons[self->nicons-1]);
                    self->nicons--;
                }
                xerror_set_ignore(FALSE);
            }
            XFree(hints);
        }
    }

    if (self->frame)
	frame_adjust_icon(self->frame);
}

static void client_change_state(Client *self)
{
    guint32 state[2];
    guint32 netstate[10];
    guint num;

    state[0] = self->wmstate;
    state[1] = None;
    PROP_SETA32(self->window, wm_state, wm_state, state, 2);

    num = 0;
    if (self->modal)
	netstate[num++] = prop_atoms.net_wm_state_modal;
    if (self->shaded)
	netstate[num++] = prop_atoms.net_wm_state_shaded;
    if (self->iconic)
	netstate[num++] = prop_atoms.net_wm_state_hidden;
    if (self->skip_taskbar)
	netstate[num++] = prop_atoms.net_wm_state_skip_taskbar;
    if (self->skip_pager)
	netstate[num++] = prop_atoms.net_wm_state_skip_pager;
    if (self->fullscreen)
	netstate[num++] = prop_atoms.net_wm_state_fullscreen;
    if (self->max_vert)
	netstate[num++] = prop_atoms.net_wm_state_maximized_vert;
    if (self->max_horz)
	netstate[num++] = prop_atoms.net_wm_state_maximized_horz;
    if (self->above)
	netstate[num++] = prop_atoms.net_wm_state_above;
    if (self->below)
	netstate[num++] = prop_atoms.net_wm_state_below;
    PROP_SETA32(self->window, net_wm_state, atom, netstate, num);

    client_calc_layer(self);

    if (self->frame)
	frame_adjust_state(self->frame);
}

Client *client_search_focus_tree(Client *self)
{
    GSList *it;
    Client *ret;

    for (it = self->transients; it != NULL; it = it->next) {
	if (client_focused(it->data)) return it->data;
	if ((ret = client_search_focus_tree(it->data))) return ret;
    }
    return NULL;
}

Client *client_search_focus_tree_full(Client *self)
{
    if (self->transient_for) {
        if (self->transient_for != TRAN_GROUP) {
            return client_search_focus_tree_full(self->transient_for);
        } else {
            GSList *it;
            gboolean recursed = FALSE;
        
            for (it = self->group->members; it; it = it->next)
                if (!((Client*)it->data)->transient_for) {
                    Client *c;
                    if ((c = client_search_focus_tree_full(it->data)))
                        return c;
                    recursed = TRUE;
                }
            if (recursed)
              return NULL;
        }
    }

    /* this function checks the whole tree, the client_search_focus_tree~
       does not, so we need to check this window */
    if (client_focused(self))
      return self;
    return client_search_focus_tree(self);
}

static StackLayer calc_layer(Client *self)
{
    StackLayer l;

    if (self->fullscreen) l = Layer_Fullscreen;
    else if (self->type == Type_Desktop) l = Layer_Desktop;
    else if (self->type == Type_Dock) {
        if (!self->below) l = Layer_Top;
        else l = Layer_Normal;
    }
    else if (self->above) l = Layer_Above;
    else if (self->below) l = Layer_Below;
    else l = Layer_Normal;

    return l;
}

static void calc_recursive(Client *self, Client *orig, StackLayer l,
                           gboolean raised)
{
    StackLayer old, own;
    GSList *it;

    old = self->layer;
    own = calc_layer(self);
    self->layer = l > own ? l : own;

    for (it = self->transients; it; it = it->next)
        calc_recursive(it->data, orig, l, raised ? raised : l != old);

    if (!raised && l != old)
	if (orig->frame) /* only restack if the original window is managed */
	    stacking_raise(CLIENT_AS_WINDOW(self));
}

void client_calc_layer(Client *self)
{
    StackLayer l;
    Client *orig;

    orig = self;

    /* transients take on the layer of their parents */
    if (self->transient_for) {
        if (self->transient_for != TRAN_GROUP) {
            self = self->transient_for;
        } else {
            GSList *it;

            for (it = self->group->members; it; it = it->next)
                if (it->data != self &&
                    !((Client*)it->data)->transient_for) {
                    self = it->data;
                    break;
                }
        }
    }

    l = calc_layer(self);

    calc_recursive(self, orig, l, FALSE);
}

gboolean client_should_show(Client *self)
{
    if (self->iconic) return FALSE;
    else if (!(self->desktop == screen_desktop ||
	       self->desktop == DESKTOP_ALL)) return FALSE;
    else if (client_normal(self) && screen_showing_desktop) return FALSE;
    
    return TRUE;
}

static void client_showhide(Client *self)
{

    if (client_should_show(self))
        frame_show(self->frame);
    else
        frame_hide(self->frame);
}

gboolean client_normal(Client *self) {
    return ! (self->type == Type_Desktop || self->type == Type_Dock ||
	      self->type == Type_Splash);
}

static void client_apply_startup_state(Client *self)
{
    /* these are in a carefully crafted order.. */

    if (self->iconic) {
	self->iconic = FALSE;
	client_iconify(self, TRUE, FALSE);
    }
    if (self->fullscreen) {
	self->fullscreen = FALSE;
	client_fullscreen(self, TRUE, FALSE);
    }
    if (self->shaded) {
	self->shaded = FALSE;
	client_shade(self, TRUE);
    }
    if (self->urgent)
        dispatch_client(Event_Client_Urgent, self, self->urgent, 0);
  
    if (self->max_vert && self->max_horz) {
	self->max_vert = self->max_horz = FALSE;
	client_maximize(self, TRUE, 0, FALSE);
    } else if (self->max_vert) {
	self->max_vert = FALSE;
	client_maximize(self, TRUE, 2, FALSE);
    } else if (self->max_horz) {
	self->max_horz = FALSE;
	client_maximize(self, TRUE, 1, FALSE);
    }

    /* nothing to do for the other states:
       skip_taskbar
       skip_pager
       modal
       above
       below
    */
}

void client_configure(Client *self, Corner anchor, int x, int y, int w, int h,
		      gboolean user, gboolean final)
{
    gboolean moved = FALSE, resized = FALSE;

    /* gets the frame's position */
    frame_client_gravity(self->frame, &x, &y);

    /* these positions are frame positions, not client positions */

    /* set the size and position if fullscreen */
    if (self->fullscreen) {
#ifdef VIDMODE
        int dot;
        XF86VidModeModeLine mode;

        XF86VidModeGetViewPort(ob_display, ob_screen, &x, &y);
        XF86VidModeGetModeLine(ob_display, ob_screen, &dot, &mode);
        w = mode.hdisplay;
        h = mode.vdisplay;
        if (mode.privsize) XFree(mode.private);
#else
	x = 0;
	y = 0;
	w = screen_physical_size.width;
	h = screen_physical_size.height;
#endif
        user = FALSE; /* ignore that increment etc shit when in fullscreen */
    } else {
        /* set the size and position if maximized */
        if (self->max_horz) {
            x = screen_area(self->desktop)->x - self->frame->size.left;
            w = screen_area(self->desktop)->width;
        }
        if (self->max_vert) {
            y = screen_area(self->desktop)->y;
            h = screen_area(self->desktop)->height -
                self->frame->size.top - self->frame->size.bottom;
        }
    }

    /* gets the client's position */
    frame_frame_gravity(self->frame, &x, &y);

    /* these override the above states! if you cant move you can't move! */
    if (user) {
        if (!(self->functions & Func_Move)) {
            x = self->area.x;
            y = self->area.y;
        }
        if (!(self->functions & Func_Resize)) {
            w = self->area.width;
            h = self->area.height;
        }
    }

    if (!(w == self->area.width && h == self->area.height)) {
        w -= self->base_size.width;
        h -= self->base_size.height;

        if (user) {
            /* for interactive resizing. have to move half an increment in each
               direction. */

            /* how far we are towards the next size inc */
            int mw = w % self->size_inc.width; 
            int mh = h % self->size_inc.height;
            /* amount to add */
            int aw = self->size_inc.width / 2;
            int ah = self->size_inc.height / 2;
            /* don't let us move into a new size increment */
            if (mw + aw >= self->size_inc.width)
                aw = self->size_inc.width - mw - 1;
            if (mh + ah >= self->size_inc.height)
                ah = self->size_inc.height - mh - 1;
            w += aw;
            h += ah;
    
            /* if this is a user-requested resize, then check against min/max
               sizes and aspect ratios */

            /* smaller than min size or bigger than max size? */
            if (w > self->max_size.width) w = self->max_size.width;
            if (w < self->min_size.width) w = self->min_size.width;
            if (h > self->max_size.height) h = self->max_size.height;
            if (h < self->min_size.height) h = self->min_size.height;

            /* adjust the height ot match the width for the aspect ratios */
            if (self->min_ratio)
                if (h * self->min_ratio > w) h = (int)(w / self->min_ratio);
            if (self->max_ratio)
                if (h * self->max_ratio < w) h = (int)(w / self->max_ratio);
        }

        /* keep to the increments */
        w /= self->size_inc.width;
        h /= self->size_inc.height;

        /* you cannot resize to nothing */
        if (w < 1) w = 1;
        if (h < 1) h = 1;
  
        /* store the logical size */
        SIZE_SET(self->logical_size, w, h);

        w *= self->size_inc.width;
        h *= self->size_inc.height;

        w += self->base_size.width;
        h += self->base_size.height;
    }

    switch (anchor) {
    case Corner_TopLeft:
	break;
    case Corner_TopRight:
	x -= w - self->area.width;
	break;
    case Corner_BottomLeft:
	y -= h - self->area.height;
	break;
    case Corner_BottomRight:
	x -= w - self->area.width;
	y -= h - self->area.height;
	break;
    }

    moved = x != self->area.x || y != self->area.y;
    resized = w != self->area.width || h != self->area.height;

    RECT_SET(self->area, x, y, w, h);

    if (resized)
	XResizeWindow(ob_display, self->window, w, h);

    /* move/resize the frame to match the request */
    if (self->frame) {
        if (moved || resized)
            frame_adjust_area(self->frame, moved, resized);

        if (!user || final) {
            XEvent event;
            event.type = ConfigureNotify;
            event.xconfigure.display = ob_display;
            event.xconfigure.event = self->window;
            event.xconfigure.window = self->window;
    
            /* root window real coords */
            event.xconfigure.x = self->frame->area.x + self->frame->size.left;
            event.xconfigure.y = self->frame->area.y + self->frame->size.top;
    
            event.xconfigure.width = w;
            event.xconfigure.height = h;
            event.xconfigure.border_width = 0;
            event.xconfigure.above = self->frame->plate;
            event.xconfigure.override_redirect = FALSE;
            XSendEvent(event.xconfigure.display, event.xconfigure.window,
                       FALSE, StructureNotifyMask, &event);
	}
    }
}

void client_fullscreen(Client *self, gboolean fs, gboolean savearea)
{
    int x, y, w, h;

    if (!(self->functions & Func_Fullscreen) || /* can't */
	self->fullscreen == fs) return;         /* already done */

    self->fullscreen = fs;
    client_change_state(self); /* change the state hints on the client,
                                  and adjust out layer/stacking */

    if (fs) {
	if (savearea) {
	    guint32 dimensions[4];
	    dimensions[0] = self->area.x;
	    dimensions[1] = self->area.y;
	    dimensions[2] = self->area.width;
	    dimensions[3] = self->area.height;
  
	    PROP_SETA32(self->window, openbox_premax, cardinal,
			dimensions, 4);
	}

        /* these are not actually used cuz client_configure will set them
           as appropriate when the window is fullscreened */
        x = y = w = h = 0;
    } else {
        guint num;
	gint32 *dimensions;

        /* pick some fallbacks... */
        x = screen_area(self->desktop)->x +
            screen_area(self->desktop)->width / 4;
        y = screen_area(self->desktop)->y +
            screen_area(self->desktop)->height / 4;
        w = screen_area(self->desktop)->width / 2;
        h = screen_area(self->desktop)->height / 2;

	if (PROP_GETA32(self->window, openbox_premax, cardinal,
                        (guint32**)&dimensions, &num)) {
            if (num == 4) {
                x = dimensions[0];
                y = dimensions[1];
                w = dimensions[2];
                h = dimensions[3];
            }
	    g_free(dimensions);
	}
    }

    client_setup_decor_and_functions(self);

    client_configure(self, Corner_TopLeft, x, y, w, h, TRUE, TRUE);

    /* try focus us when we go into fullscreen mode */
    client_focus(self);
}

void client_iconify(Client *self, gboolean iconic, gboolean curdesk)
{
    GSList *it;

    /* move up the transient chain as far as possible first */
    if (self->transient_for) {
        if (self->transient_for != TRAN_GROUP) {
            if (self->transient_for->iconic != iconic) {
                client_iconify(self->transient_for, iconic, curdesk);
                return;
            }
        } else {
            GSList *it;

            for (it = self->group->members; it; it = it->next) {
                Client *c = it->data;
                if (c != self && c->iconic != iconic &&
                    !c->transient_for) {
                    client_iconify(it->data, iconic, curdesk);
                    break;
                }
            }
            if (it != NULL) return;
        }
    }

    if (self->iconic == iconic) return; /* nothing to do */

    g_message("%sconifying window: 0x%lx", (iconic ? "I" : "Uni"),
	      self->window);

    self->iconic = iconic;

    if (iconic) {
	self->wmstate = IconicState;
	self->ignore_unmaps++;
	/* we unmap the client itself so that we can get MapRequest events,
	   and because the ICCCM tells us to! */
	XUnmapWindow(ob_display, self->window);

        /* update the focus lists.. iconic windows go to the bottom of the
         list, put the new iconic window at the 'top of the bottom'. */
        focus_order_to_top(self);
    } else {
	if (curdesk)
	    client_set_desktop(self, screen_desktop, FALSE);
	self->wmstate = self->shaded ? IconicState : NormalState;
	XMapWindow(ob_display, self->window);

        /* this puts it after the current focused window */
        focus_order_remove(self);
        focus_order_add_new(self);

        /* this is here cuz with the VIDMODE extension, the viewport can change
           while a fullscreen window is iconic, and when it uniconifies, it
           would be nice if it did so to the new position of the viewport */
        client_reconfigure(self);
    }
    client_change_state(self);
    client_showhide(self);
    screen_update_struts();

    dispatch_client(iconic ? Event_Client_Unmapped : Event_Client_Mapped,
                    self, 0, 0);

    /* iconify all transients */
    for (it = self->transients; it != NULL; it = it->next)
        if (it->data != self) client_iconify(it->data, iconic, curdesk);
}

void client_maximize(Client *self, gboolean max, int dir, gboolean savearea)
{
    int x, y, w, h;
     
    g_assert(dir == 0 || dir == 1 || dir == 2);
    if (!(self->functions & Func_Maximize)) return; /* can't */

    /* check if already done */
    if (max) {
	if (dir == 0 && self->max_horz && self->max_vert) return;
	if (dir == 1 && self->max_horz) return;
	if (dir == 2 && self->max_vert) return;
    } else {
	if (dir == 0 && !self->max_horz && !self->max_vert) return;
	if (dir == 1 && !self->max_horz) return;
	if (dir == 2 && !self->max_vert) return;
    }

    /* work with the frame's coords */
    x = self->frame->area.x;
    y = self->frame->area.y;
    w = self->area.width;
    h = self->area.height;

    if (max) {
	if (savearea) {
	    gint32 dimensions[4];
	    gint32 *readdim;
            guint num;

	    dimensions[0] = x;
	    dimensions[1] = y;
	    dimensions[2] = w;
	    dimensions[3] = h;

	    /* get the property off the window and use it for the dimensions
	       we are already maxed on */
	    if (PROP_GETA32(self->window, openbox_premax, cardinal,
			    (guint32**)&readdim, &num)) {
                if (num == 4) {
                    if (self->max_horz) {
                        dimensions[0] = readdim[0];
                        dimensions[2] = readdim[2];
                    }
                    if (self->max_vert) {
                        dimensions[1] = readdim[1];
                        dimensions[3] = readdim[3];
                    }
                }
		g_free(readdim);
	    }

	    PROP_SETA32(self->window, openbox_premax, cardinal,
			(guint32*)dimensions, 4);
	}
    } else {
        guint num;
	gint32 *dimensions;

        /* pick some fallbacks... */
        if (dir == 0 || dir == 1) { /* horz */
            x = screen_area(self->desktop)->x +
                screen_area(self->desktop)->width / 4;
            w = screen_area(self->desktop)->width / 2;
        }
        if (dir == 0 || dir == 2) { /* vert */
            y = screen_area(self->desktop)->y +
                screen_area(self->desktop)->height / 4;
            h = screen_area(self->desktop)->height / 2;
        }

	if (PROP_GETA32(self->window, openbox_premax, cardinal,
			(guint32**)&dimensions, &num)) {
            if (num == 4) {
                if (dir == 0 || dir == 1) { /* horz */
                    x = dimensions[0];
                    w = dimensions[2];
                }
                if (dir == 0 || dir == 2) { /* vert */
                    y = dimensions[1];
                    h = dimensions[3];
                }
            }
            g_free(dimensions);
	}
    }

    if (dir == 0 || dir == 1) /* horz */
	self->max_horz = max;
    if (dir == 0 || dir == 2) /* vert */
	self->max_vert = max;

    if (!self->max_horz && !self->max_vert)
	PROP_ERASE(self->window, openbox_premax);

    client_change_state(self); /* change the state hints on the client */

    /* figure out where the client should be going */
    frame_frame_gravity(self->frame, &x, &y);
    client_configure(self, Corner_TopLeft, x, y, w, h, TRUE, TRUE);
}

void client_shade(Client *self, gboolean shade)
{
    if ((!(self->functions & Func_Shade) && shade) || /* can't shade */
	self->shaded == shade) return;     /* already done */

    /* when we're iconic, don't change the wmstate */
    if (!self->iconic)
	self->wmstate = shade ? IconicState : NormalState;
    self->shaded = shade;
    client_change_state(self);
    /* resize the frame to just the titlebar */
    frame_adjust_area(self->frame, FALSE, FALSE);
}

void client_close(Client *self)
{
    XEvent ce;

    if (!(self->functions & Func_Close)) return;

    /*
      XXX: itd be cool to do timeouts and shit here for killing the client's
      process off
      like... if the window is around after 5 seconds, then the close button
      turns a nice red, and if this function is called again, the client is
      explicitly killed.
    */

    ce.xclient.type = ClientMessage;
    ce.xclient.message_type =  prop_atoms.wm_protocols;
    ce.xclient.display = ob_display;
    ce.xclient.window = self->window;
    ce.xclient.format = 32;
    ce.xclient.data.l[0] = prop_atoms.wm_delete_window;
    ce.xclient.data.l[1] = event_lasttime;
    ce.xclient.data.l[2] = 0l;
    ce.xclient.data.l[3] = 0l;
    ce.xclient.data.l[4] = 0l;
    XSendEvent(ob_display, self->window, FALSE, NoEventMask, &ce);
}

void client_kill(Client *self)
{
    XKillClient(ob_display, self->window);
}

void client_set_desktop(Client *self, guint target, gboolean donthide)
{
    guint old;

    if (target == self->desktop) return;
  
    g_message("Setting desktop %u", target+1);

    g_assert(target < screen_num_desktops || target == DESKTOP_ALL);

    /* remove from the old desktop(s) */
    focus_order_remove(self);

    old = self->desktop;
    self->desktop = target;
    PROP_SET32(self->window, net_wm_desktop, cardinal, target);
    /* the frame can display the current desktop state */
    frame_adjust_state(self->frame);
    /* 'move' the window to the new desktop */
    if (!donthide)
        client_showhide(self);
    /* raise if it was not already on the desktop */
    if (old != DESKTOP_ALL)
        stacking_raise(CLIENT_AS_WINDOW(self));
    screen_update_struts();

    /* add to the new desktop(s) */
    if (config_focus_new)
        focus_order_to_top(self);
    else
        focus_order_to_bottom(self);

    dispatch_client(Event_Client_Desktop, self, target, old);
}

Client *client_search_modal_child(Client *self)
{
    GSList *it;
    Client *ret;
  
    for (it = self->transients; it != NULL; it = it->next) {
	Client *c = it->data;
	if ((ret = client_search_modal_child(c))) return ret;
	if (c->modal) return c;
    }
    return NULL;
}

gboolean client_validate(Client *self)
{
    XEvent e; 

    XSync(ob_display, FALSE); /* get all events on the server */

    if (XCheckTypedWindowEvent(ob_display, self->window, DestroyNotify, &e) ||
	XCheckTypedWindowEvent(ob_display, self->window, UnmapNotify, &e)) {
	XPutBackEvent(ob_display, &e);
	return FALSE;
    }

    return TRUE;
}

void client_set_wm_state(Client *self, long state)
{
    if (state == self->wmstate) return; /* no change */
  
    switch (state) {
    case IconicState:
	client_iconify(self, TRUE, TRUE);
	break;
    case NormalState:
	client_iconify(self, FALSE, TRUE);
	break;
    }
}

void client_set_state(Client *self, Atom action, long data1, long data2)
{
    gboolean shaded = self->shaded;
    gboolean fullscreen = self->fullscreen;
    gboolean max_horz = self->max_horz;
    gboolean max_vert = self->max_vert;
    int i;

    if (!(action == prop_atoms.net_wm_state_add ||
	  action == prop_atoms.net_wm_state_remove ||
	  action == prop_atoms.net_wm_state_toggle))
	/* an invalid action was passed to the client message, ignore it */
	return; 

    for (i = 0; i < 2; ++i) {
	Atom state = i == 0 ? data1 : data2;
    
	if (!state) continue;

	/* if toggling, then pick whether we're adding or removing */
	if (action == prop_atoms.net_wm_state_toggle) {
	    if (state == prop_atoms.net_wm_state_modal)
		action = self->modal ? prop_atoms.net_wm_state_remove :
		    prop_atoms.net_wm_state_add;
	    else if (state == prop_atoms.net_wm_state_maximized_vert)
		action = self->max_vert ? prop_atoms.net_wm_state_remove :
		    prop_atoms.net_wm_state_add;
	    else if (state == prop_atoms.net_wm_state_maximized_horz)
		action = self->max_horz ? prop_atoms.net_wm_state_remove :
		    prop_atoms.net_wm_state_add;
	    else if (state == prop_atoms.net_wm_state_shaded)
		action = self->shaded ? prop_atoms.net_wm_state_remove :
		    prop_atoms.net_wm_state_add;
	    else if (state == prop_atoms.net_wm_state_skip_taskbar)
		action = self->skip_taskbar ?
		    prop_atoms.net_wm_state_remove :
		    prop_atoms.net_wm_state_add;
	    else if (state == prop_atoms.net_wm_state_skip_pager)
		action = self->skip_pager ?
		    prop_atoms.net_wm_state_remove :
		    prop_atoms.net_wm_state_add;
	    else if (state == prop_atoms.net_wm_state_fullscreen)
		action = self->fullscreen ?
		    prop_atoms.net_wm_state_remove :
		    prop_atoms.net_wm_state_add;
	    else if (state == prop_atoms.net_wm_state_above)
		action = self->above ? prop_atoms.net_wm_state_remove :
		    prop_atoms.net_wm_state_add;
	    else if (state == prop_atoms.net_wm_state_below)
		action = self->below ? prop_atoms.net_wm_state_remove :
		    prop_atoms.net_wm_state_add;
	}
    
	if (action == prop_atoms.net_wm_state_add) {
	    if (state == prop_atoms.net_wm_state_modal) {
		/* XXX raise here or something? */
		self->modal = TRUE;
	    } else if (state == prop_atoms.net_wm_state_maximized_vert) {
		max_vert = TRUE;
	    } else if (state == prop_atoms.net_wm_state_maximized_horz) {
		max_horz = TRUE;
	    } else if (state == prop_atoms.net_wm_state_shaded) {
		shaded = TRUE;
	    } else if (state == prop_atoms.net_wm_state_skip_taskbar) {
		self->skip_taskbar = TRUE;
	    } else if (state == prop_atoms.net_wm_state_skip_pager) {
		self->skip_pager = TRUE;
	    } else if (state == prop_atoms.net_wm_state_fullscreen) {
		fullscreen = TRUE;
	    } else if (state == prop_atoms.net_wm_state_above) {
		self->above = TRUE;
	    } else if (state == prop_atoms.net_wm_state_below) {
		self->below = TRUE;
	    }

	} else { /* action == prop_atoms.net_wm_state_remove */
	    if (state == prop_atoms.net_wm_state_modal) {
		self->modal = FALSE;
	    } else if (state == prop_atoms.net_wm_state_maximized_vert) {
		max_vert = FALSE;
	    } else if (state == prop_atoms.net_wm_state_maximized_horz) {
		max_horz = FALSE;
	    } else if (state == prop_atoms.net_wm_state_shaded) {
		shaded = FALSE;
	    } else if (state == prop_atoms.net_wm_state_skip_taskbar) {
		self->skip_taskbar = FALSE;
	    } else if (state == prop_atoms.net_wm_state_skip_pager) {
		self->skip_pager = FALSE;
	    } else if (state == prop_atoms.net_wm_state_fullscreen) {
		fullscreen = FALSE;
	    } else if (state == prop_atoms.net_wm_state_above) {
		self->above = FALSE;
	    } else if (state == prop_atoms.net_wm_state_below) {
		self->below = FALSE;
	    }
	}
    }
    if (max_horz != self->max_horz || max_vert != self->max_vert) {
	if (max_horz != self->max_horz && max_vert != self->max_vert) {
	    /* toggling both */
	    if (max_horz == max_vert) { /* both going the same way */
		client_maximize(self, max_horz, 0, TRUE);
	    } else {
		client_maximize(self, max_horz, 1, TRUE);
		client_maximize(self, max_vert, 2, TRUE);
	    }
	} else {
	    /* toggling one */
	    if (max_horz != self->max_horz)
		client_maximize(self, max_horz, 1, TRUE);
	    else
		client_maximize(self, max_vert, 2, TRUE);
	}
    }
    /* change fullscreen state before shading, as it will affect if the window
       can shade or not */
    if (fullscreen != self->fullscreen)
	client_fullscreen(self, fullscreen, TRUE);
    if (shaded != self->shaded)
	client_shade(self, shaded);
    client_calc_layer(self);
    client_change_state(self); /* change the hint to reflect these changes */
}

Client *client_focus_target(Client *self)
{
    Client *child;
     
    /* if we have a modal child, then focus it, not us */
    child = client_search_modal_child(self);
    if (child) return child;
    return self;
}

gboolean client_can_focus(Client *self)
{
    XEvent ev;

    /* choose the correct target */
    self = client_focus_target(self);

    if (!self->frame->visible)
        return FALSE;

    if (!((self->can_focus || self->focus_notify) &&
          (self->desktop == screen_desktop ||
           self->desktop == DESKTOP_ALL) &&
          !self->iconic))
	return FALSE;

    /* do a check to see if the window has already been unmapped or destroyed
       do this intelligently while watching out for unmaps we've generated
       (ignore_unmaps > 0) */
    if (XCheckTypedWindowEvent(ob_display, self->window,
			       DestroyNotify, &ev)) {
	XPutBackEvent(ob_display, &ev);
	return FALSE;
    }
    while (XCheckTypedWindowEvent(ob_display, self->window,
				  UnmapNotify, &ev)) {
	if (self->ignore_unmaps) {
	    self->ignore_unmaps--;
	} else {
	    XPutBackEvent(ob_display, &ev);
	    return FALSE;
	}
    }

    return TRUE;
}

gboolean client_focus(Client *self)
{
    /* choose the correct target */
    self = client_focus_target(self);

    if (!client_can_focus(self)) {
        if (!self->frame->visible) {
            /* update the focus lists */
            focus_order_to_top(self);
        }
        return FALSE;
    }

    if (self->can_focus)
        /* RevertToPointerRoot causes much more headache than RevertToNone, so
           I choose to use it always, hopefully to find errors quicker, if any
           are left. (I hate X. I hate focus events.) */
	XSetInputFocus(ob_display, self->window, RevertToPointerRoot,
                       event_lasttime);

    if (self->focus_notify) {
	XEvent ce;
	ce.xclient.type = ClientMessage;
	ce.xclient.message_type = prop_atoms.wm_protocols;
	ce.xclient.display = ob_display;
	ce.xclient.window = self->window;
	ce.xclient.format = 32;
	ce.xclient.data.l[0] = prop_atoms.wm_take_focus;
	ce.xclient.data.l[1] = event_lasttime;
	ce.xclient.data.l[2] = 0l;
	ce.xclient.data.l[3] = 0l;
	ce.xclient.data.l[4] = 0l;
	XSendEvent(ob_display, self->window, FALSE, NoEventMask, &ce);
    }

#ifdef DEBUG_FOCUS
    g_message("%sively focusing %lx at %d", (self->can_focus ? "act" : "pass"),
              self->window, (int)
              event_lasttime);
#endif

    /* Cause the FocusIn to come back to us. Important for desktop switches,
       since otherwise we'll have no FocusIn on the queue and send it off to
       the focus_backup. */
    XSync(ob_display, FALSE);
    return TRUE;
}

void client_unfocus(Client *self)
{
    g_assert(focus_client == self);
#ifdef DEBUG_FOCUS
    g_message("client_unfocus for %lx", self->window);
#endif
    focus_fallback(Fallback_Unfocusing);
}

void client_activate(Client *self)
{
    if (client_normal(self) && screen_showing_desktop)
        screen_show_desktop(FALSE);
    if (self->iconic)
        client_iconify(self, FALSE, TRUE);
    else if (!self->frame->visible)
        /* if its not visible for other reasons, then don't mess
           with it */
        return;
    if (self->shaded)
        client_shade(self, FALSE);
    client_focus(self);
    stacking_raise(CLIENT_AS_WINDOW(self));
}

gboolean client_focused(Client *self)
{
    return self == focus_client;
}

Icon *client_icon(Client *self, int w, int h)
{
    int i;
    /* si is the smallest image >= req */
    /* li is the largest image < req */
    unsigned long size, smallest = 0xffffffff, largest = 0, si = 0, li = 0;

    if (!self->nicons) return NULL;

    for (i = 0; i < self->nicons; ++i) {
        size = self->icons[i].width * self->icons[i].height;
        if (size < smallest && size >= (unsigned)(w * h)) {
            smallest = size;
            si = i;
        }
        if (size > largest && size <= (unsigned)(w * h)) {
            largest = size;
            li = i;
        }
    }
    if (largest == 0) /* didnt find one smaller than the requested size */
        return &self->icons[si];
    return &self->icons[li];
}

/* this be mostly ripped from fvwm */
Client *client_find_directional(Client *c, Direction dir) 
{
    int my_cx, my_cy, his_cx, his_cy;
    int offset = 0;
    int distance = 0;
    int score, best_score;
    Client *best_client, *cur;
    GList *it;

    if(!client_list)
        return NULL;

    /* first, find the centre coords of the currently focused window */
    my_cx = c->frame->area.x + c->frame->area.width / 2;
    my_cy = c->frame->area.y + c->frame->area.height / 2;

    best_score = -1;
    best_client = NULL;

    for(it = g_list_first(client_list); it; it = it->next) {
        cur = it->data;

        /* the currently selected window isn't interesting */
        if(cur == c)
            continue;
        if (!client_normal(cur))
            continue;
        if(c->desktop != cur->desktop && cur->desktop != DESKTOP_ALL)
            continue;
        if(cur->iconic)
            continue;
	if(client_focus_target(cur) == cur &&
           !(cur->can_focus || cur->focus_notify))
            continue;

        /* find the centre coords of this window, from the
         * currently focused window's point of view */
        his_cx = (cur->frame->area.x - my_cx)
            + cur->frame->area.width / 2;
        his_cy = (cur->frame->area.y - my_cy)
            + cur->frame->area.height / 2;

        if(dir > 3) { 
            int tx;
            /* Rotate the diagonals 45 degrees counterclockwise.
             * To do this, multiply the matrix /+h +h\ with the
             * vector (x y).                   \-h +h/
             * h = sqrt(0.5). We can set h := 1 since absolute
             * distance doesn't matter here. */
            tx = his_cx + his_cy;
            his_cy = -his_cx + his_cy;
            his_cx = tx;
        }

        switch(dir) {
        case Direction_North :
        case Direction_South :
        case Direction_NorthEast :
        case Direction_SouthWest :
            offset = (his_cx < 0) ? -his_cx : his_cx;
            distance = (dir == Direction_North || dir == Direction_NorthEast) ?
                -his_cy : his_cy;
            break;
        case Direction_East :
        case Direction_West :
        case Direction_SouthEast :
        case Direction_NorthWest :
            offset = (his_cy < 0) ? -his_cy : his_cy;
            distance = (dir == Direction_West || dir == Direction_NorthWest) ?
                -his_cx : his_cx;
            break;
        }

        /* the target must be in the requested direction */
        if(distance <= 0)
            continue;

        /* Calculate score for this window.  The smaller the better. */
        score = distance + offset;

        /* windows more than 45 degrees off the direction are
         * heavily penalized and will only be chosen if nothing
         * else within a million pixels */
        if(offset > distance)
            score += 1000000;

        if(best_score == -1 || score < best_score)
            best_client = cur,
                best_score = score;
    }

    return best_client;
}

void client_set_layer(Client *self, int layer)
{
    if (layer < 0) {
        self->below = TRUE;
        self->above = FALSE;
    } else if (layer == 0) {
        self->below = self->above = FALSE;
    } else {
        self->below = FALSE;
        self->above = TRUE;
    }
    client_calc_layer(self);
    client_change_state(self); /* reflect this in the state hints */
}
