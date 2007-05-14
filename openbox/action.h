/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   action.h for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003-2007   Dana Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#ifndef __action_h
#define __action_h

#include "misc.h"
#include "frame.h"
#include "parser/parse.h"

struct _ObClient;

typedef struct _ObAction ObAction;

/* These have to all have a Client* at the top even if they don't use it, so
   that I can set it blindly later on. So every function will have a Client*
   available (possibly NULL though) if it wants it.
*/

typedef enum
{
    OB_CLIENT_ACTION_NO,
    OB_CLIENT_ACTION_OPTIONAL,
    OB_CLIENT_ACTION_ALWAYS
} ObClientActionReq;

struct AnyAction {
    ObClientActionReq client_action;
    struct _ObClient *c;
    ObFrameContext context;
    gboolean interactive;
    gint x;
    gint y;
    gint button;
    Time time;
};

struct InteractiveAction {
    struct AnyAction any;
    gboolean final;
    gboolean cancel;
};

struct InterDirectionalAction{
    struct InteractiveAction inter;
    ObDirection direction;
    gboolean dialog;
    gboolean dock_windows;
    gboolean desktop_windows;
};

struct DirectionalAction{
    struct AnyAction any;
    ObDirection direction;
    gboolean hang;
};

struct Debug {
    gchar *string;
};

struct Execute {
    struct AnyAction any;
    gchar *path;
    gboolean startupnotify;
    gchar *name;
    gchar *icon_name;
};

struct ClientAction {
    struct AnyAction any;
};

struct Activate {
    struct AnyAction any;
    gboolean here; /* bring it to the current desktop */
};

struct MoveResizeRelative {
    struct AnyAction any;
    gint deltax;
    gint deltay;
    gint deltaxl;
    gint deltayu;
};

struct SendToDesktop {
    struct AnyAction any;
    guint desk;
    gboolean follow;
};

struct SendToDesktopDirection {
    struct InteractiveAction inter;
    ObDirection dir;
    gboolean wrap;
    gboolean linear;
    gboolean follow;
};

struct Desktop {
    struct InteractiveAction inter;
    guint desk;
};

struct Layer {
    struct AnyAction any;
    gint layer; /* < 0 = below, 0 = normal, > 0 = above */
};

struct DesktopDirection {
    struct InteractiveAction inter;
    ObDirection dir;
    gboolean wrap;
    gboolean linear;
};

struct MoveResize {
    struct AnyAction any;
    gboolean keyboard;
    guint32 corner;
};

struct ShowMenu {
    struct AnyAction any;
    gchar *name;
};

struct CycleWindows {
    struct InteractiveAction inter;
    gboolean linear;
    gboolean forward;
    gboolean dialog;
    gboolean dock_windows;
    gboolean desktop_windows;
    gboolean all_desktops;
};

struct Stacking {
    struct AnyAction any;
};

union ActionData {
    struct AnyAction any;
    struct InteractiveAction inter;
    struct InterDirectionalAction interdiraction;
    struct DirectionalAction diraction;
    struct Debug debug;
    struct Execute execute;
    struct ClientAction client;
    struct Activate activate;
    struct MoveResizeRelative relative;
    struct SendToDesktop sendto;
    struct SendToDesktopDirection sendtodir;
    struct Desktop desktop;
    struct DesktopDirection desktopdir;
    struct MoveResize moveresize;
    struct ShowMenu showmenu;
    struct CycleWindows cycle;
    struct Layer layer;
    struct Stacking stacking;
};

struct _ObAction {
    guint ref;

    /* The func member acts like an enum to tell which one of the structs in
       the data union are valid.
    */
    void (*func)(union ActionData *data);
    union ActionData data;
};

/* Creates a new Action from the name of the action
   A few action types need data set after making this call still. Check if
   the returned action's "func" is one of these.
   action_debug - the string to print should be set
   action_execute - the path needs to be set
   action_restart - the path can optionally be set
   action_desktop - the destination desktop needs to be set
   action_send_to_desktop - the destination desktop needs to be set
   action_move_relative_horz - the delta
   action_move_relative_vert - the delta
   action_resize_relative_horz - the delta
   action_resize_relative_vert - the delta
   action_move_relative - the deltas
   action_resize_relative - the deltas
*/

ObAction* action_from_string(const gchar *name, ObUserAction uact);
ObAction* action_parse(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                       ObUserAction uact);
void action_ref(ObAction *a);
void action_unref(ObAction *a);

ObAction* action_copy(const ObAction *a);

/*! Executes a list of actions.
  @param c The client associated with the action. Can be NULL.
  @param state The keyboard modifiers state at the time the user action occured
  @param button The mouse button used to execute the action.
  @param x The x coord at which the user action occured.
  @param y The y coord at which the user action occured.
  @param cancel If the action is cancelling an interactive action. This only
         affects interactive actions, but should generally always be FALSE.
  @param done If the action is completing an interactive action. This only
         affects interactive actions, but should generally always be FALSE.
*/
void action_run_list(GSList *acts, struct _ObClient *c, ObFrameContext context,
                     guint state, guint button, gint x, gint y, Time time,
                     gboolean cancel, gboolean done);

#define action_run_mouse(a, c, n, s, b, x, y, t) \
    action_run_list(a, c, n, s, b, x, y, t, FALSE, FALSE)

#define action_run_interactive(a, c, s, t, n, d) \
    action_run_list(a, c, OB_FRAME_CONTEXT_NONE, s, 0, -1, -1, t, n, d)

#define action_run_key(a, c, s, x, y, t) \
    action_run_list(a, c, OB_FRAME_CONTEXT_NONE, s, 0, x, y, t, FALSE, FALSE)

#define action_run(a, c, s, t) \
    action_run_list(a, c, OB_FRAME_CONTEXT_NONE, s, 0, -1, -1, t, FALSE, FALSE)

void action_run_string(const gchar *name, struct _ObClient *c, Time time);

/* Debug */
void action_debug(union ActionData *data);
/* Execute */
void action_execute(union ActionData *data);
/* ActivateAction */
void action_activate(union ActionData *data);
/* ClientAction */
void action_focus(union ActionData *data);
/* ClientAction */
void action_unfocus(union ActionData *data);
/* ClientAction */
void action_iconify(union ActionData *data);
/* ClientAction */
void action_focus_order_to_bottom(union ActionData *data);
/* ClientAction */
void action_raiselower(union ActionData *data);
/* ClientAction */
void action_raise(union ActionData *data);
/* ClientAction */
void action_lower(union ActionData *data);
/* ClientAction */
void action_close(union ActionData *data);
/* ClientAction */
void action_kill(union ActionData *data);
/* ClientAction */
void action_shade(union ActionData *data);
/* ClientAction */
void action_shadelower(union ActionData *data);
/* ClientAction */
void action_unshaderaise(union ActionData *data);
/* ClientAction */
void action_unshade(union ActionData *data);
/* ClientAction */
void action_toggle_shade(union ActionData *data);
/* ClientAction */
void action_toggle_omnipresent(union ActionData *data);
/* MoveResizeRelative */
void action_move_relative_horz(union ActionData *data);
/* MoveResizeRelative */
void action_move_relative_vert(union ActionData *data);
/* MoveResizeRelative */
void action_move_relative(union ActionData *data);
/* MoveResizeRelative */
void action_resize_relative(union ActionData *data);
/* ClientAction */
void action_move_to_center(union ActionData *data);
/* MoveResizeRelative */
void action_resize_relative_horz(union ActionData *data);
/* MoveResizeRelative */
void action_resize_relative_vert(union ActionData *data);
/* ClientAction */
void action_maximize_full(union ActionData *data);
/* ClientAction */
void action_unmaximize_full(union ActionData *data);
/* ClientAction */
void action_toggle_maximize_full(union ActionData *data);
/* ClientAction */
void action_maximize_horz(union ActionData *data);
/* ClientAction */
void action_unmaximize_horz(union ActionData *data);
/* ClientAction */
void action_toggle_maximize_horz(union ActionData *data);
/* ClientAction */
void action_maximize_vert(union ActionData *data);
/* ClientAction */
void action_unmaximize_vert(union ActionData *data);
/* ClientAction */
void action_toggle_maximize_vert(union ActionData *data);
/* ClientAction */
void action_toggle_fullscreen(union ActionData *data);
/* SendToDesktop */
void action_send_to_desktop(union ActionData *data);
/* SendToDesktopDirection */
void action_send_to_desktop_dir(union ActionData *data);
/* Desktop */
void action_desktop(union ActionData *data);
/* DesktopDirection */
void action_desktop_dir(union ActionData *data);
/* Any */
void action_desktop_last(union ActionData *data);
/* ClientAction */
void action_toggle_decorations(union ActionData *data);
/* Move */
void action_move(union ActionData *data);
/* Resize */
void action_resize(union ActionData *data);
/* Any */
void action_reconfigure(union ActionData *data);
/* Execute */
void action_restart(union ActionData *data);
/* Any */
void action_exit(union ActionData *data);
/* ShowMenu */
void action_showmenu(union ActionData *data);
/* CycleWindows */
void action_cycle_windows(union ActionData *data);
/* InterDirectionalAction */
void action_directional_focus(union ActionData *data);
/* DirectionalAction */
void action_movetoedge(union ActionData *data);
/* DirectionalAction */
void action_growtoedge(union ActionData *data);
/* Layer */
void action_send_to_layer(union ActionData *data);
/* Layer */
void action_toggle_layer(union ActionData *data);
/* Any */
void action_toggle_dockautohide(union ActionData *data);
/* Any */
void action_toggle_show_desktop(union ActionData *data);
/* Any */
void action_show_desktop(union ActionData *data);
/* Any */
void action_unshow_desktop(union ActionData *data);
/* Any */
void action_break_chroot(union ActionData *data);

#endif
