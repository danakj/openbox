#include "openbox/actions.h"
#include "openbox/client.h"

static gboolean run_func(ObActionsData *data, gpointer options);

void action_maximize_startup()
{
    actions_register("Maximize",
                     NULL, NULL,
                     run_func,
                     NULL, NULL);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    if (data->client) {
        actions_client_move(data, TRUE);
        client_maximize(data->client,
                        !(data->client->max_horz || data->client->max_vert),
                        0);
        actions_client_move(data, FALSE);
    }

    return FALSE;
}
