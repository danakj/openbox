#include "openbox/actions.h"
#include "openbox/client.h"

static gboolean run_func_on(ObActionsData *data, gpointer options);
static gboolean run_func_off(ObActionsData *data, gpointer options);
static gboolean run_func_toggle(ObActionsData *data, gpointer options);

void action_shade_startup(void)
{
    actions_register("Shade", NULL, NULL, run_func_on, NULL, NULL);
    actions_register("Unshade", NULL, NULL, run_func_off, NULL, NULL);
    actions_register("ToggleShade", NULL, NULL, run_func_toggle, NULL, NULL);
}

/* Always return FALSE because its not interactive */
static gboolean run_func_on(ObActionsData *data, gpointer options)
{
    if (data->client) {
        actions_client_move(data, TRUE);
        client_shade(data->client, TRUE);
        actions_client_move(data, FALSE);
    }
    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_off(ObActionsData *data, gpointer options)
{
    if (data->client) {
        actions_client_move(data, TRUE);
        client_shade(data->client, FALSE);
        actions_client_move(data, FALSE);
    }
    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_toggle(ObActionsData *data, gpointer options)
{
    if (data->client) {
        actions_client_move(data, TRUE);
        client_shade(data->client, !data->client->shaded);
        actions_client_move(data, FALSE);
    }
    return FALSE;
}
