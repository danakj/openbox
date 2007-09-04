#include "openbox/actions.h"
#include "openbox/prop.h"
#include "openbox/moveresize.h"

static gboolean run_func(ObActionsData *data, gpointer options);

void action_move_startup(void)
{
    actions_register("Move",
                     NULL, NULL,
                     run_func,
                     NULL, NULL);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    if (data->client) {
        guint32 corner;

        corner = data->button != 0 ?
            prop_atoms.net_wm_moveresize_move :
            prop_atoms.net_wm_moveresize_move_keyboard;

        moveresize_start(data->client, data->x, data->y, data->button, corner);
    }

    return FALSE;
}
