#include "keyaction.h"
#include <glib.h>

void keyaction_set_none(KeyAction *a, guint index)
{
    a->type[index] = DataType_Bool;
}

void keyaction_set_bool(KeyAction *a, guint index, gboolean b)
{
    a->type[index] = DataType_Bool;
    a->data[index].b = b;
}

void keyaction_set_int(KeyAction *a, guint index, int i)
{
    a->type[index] = DataType_Int;
    a->data[index].i = i;
}

void keyaction_set_uint(KeyAction *a, guint index, guint u)
{
    a->type[index] = DataType_Uint;
    a->data[index].u = u;
}

void keyaction_set_string(KeyAction *a, guint index, char *s)
{
    a->type[index] = DataType_String;
    a->data[index].s = g_strdup(s);
}

void keyaction_free(KeyAction *a)
{
    guint i;

    for (i = 0; i < 2; ++i)
        if (a->type[i] == DataType_String)
            g_free(a->data[i].s);
}

void keyaction_do(KeyAction *a, Client *c)
{
    switch (a->action) {
    case Action_Execute:
        g_assert(a->type[0] == DataType_String);
        action_execute(a->data[0].s);
        break;
    case Action_Iconify:
        if (c != NULL) action_iconify(c);
        break;
    case Action_Raise:
        if (c != NULL) action_raise(c);
        break;
    case Action_Lower:
        if (c != NULL) action_lower(c);
        break;
    case Action_Close:
        if (c != NULL) action_close(c);
        break;
    case Action_Shade:
        if (c != NULL) action_shade(c);
        break;
    case Action_Unshade:
        if (c != NULL) action_unshade(c);
        break;
    case Action_ToggleShade:
        if (c != NULL) action_toggle_shade(c);
        break;
    case Action_ToggleOmnipresent:
        if (c != NULL) action_toggle_omnipresent(c);
        break;
    case Action_MoveRelative:
        g_assert(a->type[0] == DataType_Int);
        g_assert(a->type[1] == DataType_Int);
        if (c != NULL) action_move_relative(c, a->data[0].i, a->data[1].i);
        break;
    case Action_ResizeRelative:
        g_assert(a->type[0] == DataType_Int);
        g_assert(a->type[1] == DataType_Int);
        if (c != NULL) action_resize_relative(c, a->data[0].i, a->data[1].i);
        break;
    case Action_MaximizeFull:
        if (c != NULL) action_maximize_full(c);
        break;
    case Action_UnmaximizeFull:
        if (c != NULL) action_unmaximize_full(c);
        break;
    case Action_ToggleMaximizeFull:
        if (c != NULL) action_toggle_maximize_full(c);
        break;
    case Action_MaximizeHorz:
        if (c != NULL) action_maximize_horz(c);
        break;
    case Action_UnmaximizeHorz:
        if (c != NULL) action_unmaximize_horz(c);
        break;
    case Action_ToggleMaximizeHorz:
        if (c != NULL) action_toggle_maximize_horz(c);
        break;
    case Action_MaximizeVert:
        if (c != NULL) action_maximize_vert(c);
        break;
    case Action_UnmaximizeVert:
        if (c != NULL) action_unmaximize_vert(c);
        break;
    case Action_ToggleMaximizeVert:
        if (c != NULL) action_toggle_maximize_vert(c);
        break;
    case Action_SendToDesktop:
        g_assert(a->type[0] == DataType_Uint);
        if (c != NULL) action_send_to_desktop(c, a->data[0].u);
        break;
    case Action_SendToNextDesktop:
        g_assert(a->type[0] == DataType_Bool);
        g_assert(a->type[1] == DataType_Bool);
        if (c != NULL) action_send_to_next_desktop(c, a->data[0].b,
                                                   a->data[1].b);
        break;
    case Action_SendToPreviousDesktop:
        g_assert(a->type[0] == DataType_Bool);
        g_assert(a->type[1] == DataType_Bool);
        if (c != NULL) action_send_to_previous_desktop(c, a->data[0].b,
                                                       a->data[1].b);
        break;
    case Action_Desktop:
        g_assert(a->type[0] == DataType_Uint);
        action_desktop(a->data[0].u);
        break;
    case Action_NextDesktop:
        g_assert(a->type[0] == DataType_Bool);
        action_next_desktop(a->data[0].b);
        break;
    case Action_PreviousDesktop:
        g_assert(a->type[0] == DataType_Bool);
        action_previous_desktop(a->data[0].b);
        break;
    case Action_NextDesktopColumn:
        g_assert(a->type[0] == DataType_Bool);
        action_next_desktop_column(a->data[0].b);
        break;
    case Action_PreviousDesktopColumn:
        g_assert(a->type[0] == DataType_Bool);
        action_previous_desktop_column(a->data[0].b);
        break; 
    case Action_NextDesktopRow:
        g_assert(a->type[0] == DataType_Bool);
        action_next_desktop_row(a->data[0].b);
        break;
    case Action_PreviousDesktopRow:
        g_assert(a->type[0] == DataType_Bool);
        action_previous_desktop_row(a->data[0].b);
        break; 
    case Action_ToggleDecorations:
        if (c != NULL) action_toggle_decorations(c);
        break; 
   }
}

