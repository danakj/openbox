#include "openbox/action.h"
#include "openbox/client.h"

static gboolean run_func_toggle(ObActionData *data, gpointer options);

void action_fullscreen_startup(void)
{
    action_register("ToggleFullscreen", NULL, NULL, run_func_toggle);
}

/* Always return FALSE because its not interactive */
static gboolean run_func_toggle(ObActionData *data, gpointer options)
{
    if (data->client) {
        action_client_move(data, TRUE);
        client_fullscreen(data->client, !data->client->fullscreen);
        action_client_move(data, FALSE);
    }
    return FALSE;
}
