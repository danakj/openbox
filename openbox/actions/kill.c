#include "openbox/action.h"
#include "openbox/client.h"

static gboolean run_func(ObActionData *data, gpointer options);

void action_kill_startup(void)
{
    action_register("Kill", NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionData *data, gpointer options)
{
    if (data->client)
        client_kill(data->client);

    return FALSE;
}
