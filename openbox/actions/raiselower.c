#include "openbox/actions.h"
#include "openbox/stacking.h"

static gboolean run_func(ObActionsData *data, gpointer options);

void action_raiselower_startup()
{
    actions_register("RaiseLower",
                     NULL, NULL,
                     run_func,
                     NULL, NULL);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    if (data->client) {
        actions_client_move(data, TRUE);
        stacking_restack_request(data->client, NULL, Opposite);
        actions_client_move(data, FALSE);
    }

    return FALSE;
}
