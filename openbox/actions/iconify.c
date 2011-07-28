#include "openbox/action.h"
#include "openbox/client.h"

static gboolean run_func(ObActionData *data, gpointer options);

void action_iconify_startup(void)
{
    action_register("Iconify", NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionData *data, gpointer options)
{
    if (data->client) {
        action_client_move(data, TRUE);
        client_iconify(data->client, TRUE, TRUE, FALSE);
        action_client_move(data, FALSE);
    }

    return FALSE;
}
