#include "window.h"
#include "menu.h"
#include "slit.h"
#include "client.h"
#include "frame.h"

Window window_top(ObWindow *self)
{
    switch (self->type) {
    case Window_Menu:
        return ((Menu*)self)->frame;
    case Window_Slit:
        return ((Slit*)self)->frame;
    case Window_Client:
        return ((Client*)self)->frame->window;
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
    case Window_Slit:
        return ((Slit*)self)->layer;
    case Window_Client:
        return ((Client*)self)->layer;
    case Window_Internal:
        return Layer_Internal;
    }
    g_assert_not_reached();
    return None;
}
