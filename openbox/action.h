#ifndef __action_h
#define __action_h

#include "client.h"

/* These have to all have a Client* at the top even if they don't use it, so
   that I can set it blindly later on. So every function will have a Client*
   available (possibly NULL though) if it wants it.
*/

struct AnyAction {
    Client *c;
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

struct SendToNextPreviousDesktop {
    Client *c;
    gboolean wrap;
    gboolean follow;
};

struct Desktop {
    Client *c;
    guint desk;
};

struct NextPreviousDesktop {
    Client *c;
    gboolean wrap;
};

struct Move {
    Client *c;
    int x;
    int y;
    gboolean final;
};

struct Resize {
    Client *c;
    int x;
    int y;
    gboolean final;
    Corner corner;
};

struct ShowMenu {
    Client *c;
    char * menuName;
};

union ActionData {
    struct AnyAction any;
    struct Execute execute;
    struct ClientAction client;
    struct MoveResizeRelative relative;
    struct SendToNextPreviousDesktop sendtonextprev;
    struct Desktop desktop;
    struct NextPreviousDesktop nextprevdesktop;
    struct Move move;
    struct Resize resize;
    struct ShowMenu showMenu;
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
void action_focusraise(union ActionData *data);
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
/* Desktop */
void action_send_to_desktop(union ActionData *data);
/* SendToNextPreviousDesktop */
void action_send_to_next_desktop(union ActionData *data);
/* SendToNextPreviousDesktop */
void action_send_to_previous_desktop(union ActionData *data);
/* Desktop */
void action_desktop(union ActionData *data);
/* NextPreviousDesktop */
void action_next_desktop(union ActionData *data);
/* NextPreviousDesktop */
void action_previous_desktop(union ActionData *data);
/* NextPreviousDesktop */
void action_next_desktop_column(union ActionData *data);
/* NextPreviousDesktop */
void action_previous_desktop_column(union ActionData *data);
/* NextPreviousDesktop */
void action_next_desktop_row(union ActionData *data);
/* NextPreviousDesktop */
void action_previous_desktop_row(union ActionData *data);
/* ClientAction */
void action_toggle_decorations(union ActionData *data);
/* Move */
void action_move(union ActionData *data);
/* Resize */
void action_resize(union ActionData *data);
/* Execute */
void action_restart(union ActionData *data);
/* Any */
void action_exit(union ActionData *data);
/* ShowMenu */
void action_showmenu(union ActionData *data);
#endif
