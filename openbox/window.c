#include "window.h"
#include "menu.h"
#include "config.h"
#include "dock.h"
#include "client.h"
#include "frame.h"

GHashTable *window_map;

void window_startup()
{
    window_map = g_hash_table_new(g_int_hash, g_int_equal);
}

void window_shutdown()
{
    g_hash_table_destroy(window_map);
}

Window window_top(ObWindow *self)
{
    switch (self->type) {
    case Window_Menu:
        return ((Menu*)self)->frame;
    case Window_Dock:
        return ((ObDock*)self)->frame;
    case Window_DockApp:
        /* not to be used for stacking */
        g_assert_not_reached();
        break;
    case Window_Client:
        return ((ObClient*)self)->frame->window;
    case Window_Internal:
        return ((InternalWindow*)self)->win;
    }
    g_assert_not_reached();
    return None;
}

Window window_layer(ObWindow *self)
{
    switch (self->type) {
    case Window_Menu:
        return Layer_Internal;
    case Window_Dock:
        return config_dock_layer;
    case Window_DockApp:
        /* not to be used for stacking */
        g_assert_not_reached();
        break;
    case Window_Client:
        return ((ObClient*)self)->layer;
    case Window_Internal:
        return Layer_Internal;
    }
    g_assert_not_reached();
    return None;
}
