#ifndef __action_h
#define __action_h

#include "client.h"
#include "parser/parse.h"

/* These have to all have a Client* at the top even if they don't use it, so
   that I can set it blindly later on. So every function will have a Client*
   available (possibly NULL though) if it wants it.
*/

struct AnyAction {
    Client *c;
};

struct DirectionalAction{
    Client *c;
    int direction;
};

struct Execute {
    Client *c;
    char *path;
};

struct ClientAction {
    Client *c;
};

struct MoveResizeRelative {
    Client *c;
    int delta;
};

struct SendToDesktop {
    Client *c;
    guint desk;
    gboolean follow;
};

struct SendToDesktopDirection {
    Client *c;
    gboolean wrap;
    gboolean follow;
};

struct Desktop {
    Client *c;
    guint desk;
};

struct Layer {
    Client *c;
    int layer; /* < 0 = below, 0 = normal, > 0 = above */
};

struct DesktopDirection {
    Client *c;
    gboolean wrap;
};

struct MoveResize {
    Client *c;
    int x;
    int y;
    guint32 corner; /* prop_atoms.net_wm_moveresize_* */
    guint button;
};

struct ShowMenu {
    Client *c;
    char *name;
    int x;
    int y;
};

struct CycleWindows {
    Client *c;
    gboolean linear;
    gboolean forward;
    gboolean final;
    gboolean cancel;
};

union ActionData {
    struct AnyAction any;
    struct DirectionalAction diraction;
    struct Execute execute;
    struct ClientAction client;
    struct MoveResizeRelative relative;
    struct SendToDesktop sendto;
    struct SendToDesktopDirection sendtodir;
    struct Desktop desktop;
    struct DesktopDirection desktopdir;
    struct MoveResize moveresize;
    struct ShowMenu showmenu;
    struct CycleWindows cycle;
    struct Layer layer;
};

typedef struct {
    /* The func member acts like an enum to tell which one of the structs in
       the data union are valid.
    */
    void (*func)(union ActionData *data);
    union ActionData data;
} Action;

Action *action_new(void (*func)(union ActionData *data));

/* Creates a new Action from the name of the action
   A few action types need data set after making this call still. Check if
   the returned action's "func" is one of these.
   action_execute - the path needs to be set
   action_restart - the path can optionally be set
   action_desktop - the destination desktop needs to be set
   action_move_relative_horz - the delta
   action_move_relative_vert - the delta
   action_resize_relative_horz - the delta
   action_resize_relative_vert - the delta
*/

Action *action_from_string(char *name);
Action *action_parse(xmlDocPtr doc, xmlNodePtr node);
void action_free(Action *a);

/* Execute */
void action_execute(union ActionData *data);
/* ClientAction */
void action_focus(union ActionData *data);
/* ClientAction */
void action_unfocus(union ActionData *data);
/* ClientAction */
void action_iconify(union ActionData *data);
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
/* SendToDesktop */
void action_send_to_desktop(union ActionData *data);
/* SendToDesktopDirection */
void action_send_to_desktop_right(union ActionData *data);
/* SendToDesktopDirection */
void action_send_to_desktop_left(union ActionData *data);
/* SendToDesktopDirection */
void action_send_to_desktop_up(union ActionData *data);
/* SendToDesktopDirection */
void action_send_to_desktop_down(union ActionData *data);
/* Desktop */
void action_desktop(union ActionData *data);
/* DesktopDirection */
void action_desktop_right(union ActionData *data);
/* DesktopDirection */
void action_desktop_left(union ActionData *data);
/* DesktopDirection */
void action_desktop_up(union ActionData *data);
/* DesktopDirection */
void action_desktop_down(union ActionData *data);
/* ClientAction */
void action_toggle_decorations(union ActionData *data);
/* MoveResize */
void action_moveresize(union ActionData *data);
/* Execute */
void action_restart(union ActionData *data);
/* Any */
void action_exit(union ActionData *data);
/* ShowMenu */
void action_showmenu(union ActionData *data);
/* CycleWindows */
void action_cycle_windows(union ActionData *data);
/* DirectionalAction */
void action_directional_focus(union ActionData *data);
/* DirectionalAction */
void action_movetoedge(union ActionData *data);
/* Layer */
void action_send_to_layer(union ActionData *data);
/* Layer */
void action_toggle_layer(union ActionData *data);
/* Any */
void action_toggle_show_desktop(union ActionData *data);
/* Any */
void action_show_desktop(union ActionData *data);
/* Any */
void action_unshow_desktop(union ActionData *data);

#endif
