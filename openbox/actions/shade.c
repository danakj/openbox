#include "openbox/action.h"
#include "openbox/client.h"

static gboolean run_func_on(ObActionData *data, gpointer options);
static gboolean run_func_off(ObActionData *data, gpointer options);
static gboolean run_func_toggle(ObActionData *data, gpointer options);

void action_shade_startup(void)
{
    action_register("Shade", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func_on);
    action_register("Unshade", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func_off);
    action_register("ToggleShade", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func_toggle);
}

/* Always return FALSE because its not interactive */
static gboolean run_func_on(ObActionData *data, gpointer options)
{
    if (data->client) {
        action_client_move(data, TRUE);
        client_shade(data->client, TRUE);
        action_client_move(data, FALSE);
    }
    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_off(ObActionData *data, gpointer options)
{
    if (data->client) {
        action_client_move(data, TRUE);
        client_shade(data->client, FALSE);
        action_client_move(data, FALSE);
    }
    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_toggle(ObActionData *data, gpointer options)
{
    if (data->client) {
        action_client_move(data, TRUE);
        client_shade(data->client, !data->client->shaded);
        action_client_move(data, FALSE);
    }
    return FALSE;
}
