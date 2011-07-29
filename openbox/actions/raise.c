#include "openbox/action.h"
#include "openbox/stacking.h"
#include "openbox/window.h"

static gboolean run_func(ObActionData *data, gpointer options);

void action_raise_startup(void)
{
    action_register("Raise", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionData *data, gpointer options)
{
    if (data->client) {
        action_client_move(data, TRUE);
        stacking_raise(CLIENT_AS_WINDOW(data->client));
        action_client_move(data, FALSE);
    }

    return FALSE;
}
