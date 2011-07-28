#include "openbox/action.h"
#include "openbox/stacking.h"

static gboolean run_func(ObActionData *data, gpointer options);

void action_raiselower_startup(void)
{
    action_register("RaiseLower", NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionData *data, gpointer options)
{
    if (data->client) {
        action_client_move(data, TRUE);
        stacking_restack_request(data->client, NULL, Opposite);
        action_client_move(data, FALSE);
    }

    return FALSE;
}
