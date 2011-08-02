#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/moveresize.h"
#include "obt/prop.h"

static gboolean run_func(const ObActionListRun *data, gpointer options);

void action_move_startup(void)
{
    action_register("Move", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObActionListRun *data, gpointer options)
{
    if (data->target) {
        guint32 corner;

        corner = data->pointer_button != 0 ?
            OBT_PROP_ATOM(NET_WM_MOVERESIZE_MOVE) :
            OBT_PROP_ATOM(NET_WM_MOVERESIZE_MOVE_KEYBOARD);

        moveresize_start(data->target, data->pointer_x, data->pointer_y,
                         data->pointer_button, corner);
    }

    return FALSE;
}
