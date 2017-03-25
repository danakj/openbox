#include "openbox/actions.h"
#include "openbox/client.h"

static gboolean run_func_toggle(ObActionsData *data, gpointer options);

void action_fullscreen_startup(void)
{
    actions_register("ToggleFullscreen", NULL, NULL, run_func_toggle);
    actions_register("Fullscreen", NULL, NULL, run_func_on);
    actions_register("Unfullscreen", NULL, NULL, run_func_off);
}

/* Always return FALSE because its not interactive */
static gboolean run_func_toggle(ObActionsData *data, gpointer options)
{
    if (data->client) {
        actions_client_move(data, TRUE);
        client_fullscreen(data->client, !data->client->fullscreen);
        actions_client_move(data, FALSE);
    }
    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_on(ObActionsData *data, gpointer options)
{
    if (data->client) {
        actions_client_move(data, TRUE);
        client_fullscreen(data->client, TRUE);
        actions_client_move(data, FALSE);
    }
    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_off(ObActionsData *data, gpointer options)
{
    if (data->client) {
        actions_client_move(data, TRUE);
        client_fullscreen(data->client, FALSE);
        actions_client_move(data, FALSE);
    }
    return FALSE;
}
