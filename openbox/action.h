#ifndef __action_h
#define __action_h

#include "client.h"

typedef enum {
    Action_Execute,
    Action_Iconify,
    Action_Raise,
    Action_Lower,
    Action_Close,
    Action_Shade,
    Action_Unshade,
    Action_ToggleShade,
    Action_ToggleOmnipresent,
    Action_MoveRelative,
    Action_ResizeRelative,
    Action_MaximizeFull,
    Action_UnmaximizeFull,
    Action_ToggleMaximizeFull,
    Action_MaximizeHorz,
    Action_UnmaximizeHorz,
    Action_ToggleMaximizeHorz,
    Action_MaximizeVert,
    Action_UnmaximizeVert,
    Action_ToggleMaximizeVert,
    Action_SendToDesktop,
    Action_SendToNextDesktop,
    Action_SendToPreviousDesktop,
    Action_Desktop,
    Action_NextDesktop,
    Action_PreviousDesktop,
    Action_NextDesktopColumn,
    Action_PreviousDesktopColumn,
    Action_NextDesktopRow,
    Action_PreviousDesktopRow,
    Action_ToggleDecorations
} Action;

void action_execute(char *path);
void action_iconify(Client *c);
void action_raise(Client *c);
void action_lower(Client *c);
void action_close(Client *c);
void action_shade(Client *c);
void action_unshade(Client *c);
void action_toggle_shade(Client *c);
void action_toggle_omnipresent(Client *c);
void action_move_relative(Client *c, int dx, int dy);
void action_resize_relative(Client *c, int dx, int dy);
void action_maximize_full(Client *c);
void action_unmaximize_full(Client *c);
void action_toggle_maximize_full(Client *c);
void action_maximize_horz(Client *c);
void action_unmaximize_horz(Client *c);
void action_toggle_maximize_horz(Client *c);
void action_maximize_vert(Client *c);
void action_unmaximize_vert(Client *c);
void action_toggle_maximize_vert(Client *c);
void action_send_to_desktop(Client *c, guint desktop);
void action_send_to_next_desktop(Client *c, gboolean wrap, gboolean follow);
void action_send_to_previous_desktop(Client *c, gboolean wrap,gboolean follow);
void action_desktop(guint desktop);
void action_next_desktop(gboolean wrap);
void action_previous_desktop(gboolean wrap);
void action_next_desktop_column(gboolean wrap);
void action_previous_desktop_column(gboolean wrap);
void action_next_desktop_row(gboolean wrap);
void action_previous_desktop_row(gboolean wrap);
void action_toggle_decorations(Client *c);

#endif
