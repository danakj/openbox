#include "openbox/action.h"
#include "openbox/client.h"

static gboolean run_func_on(ObActionData *data, gpointer options);
static gboolean run_func_off(ObActionData *data, gpointer options);
static gboolean run_func_toggle(ObActionData *data, gpointer options);

void action_decorations_startup(void)
{
    action_register("Decorate", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func_on);
    action_register("Undecorate", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func_off);
    action_register("ToggleDecorations", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func_toggle);
}

/* Always return FALSE because its not interactive */
static gboolean run_func_on(ObActionData *data, gpointer options)
{
    if (data->client) {
        action_client_move(data, TRUE);
        client_set_undecorated(data->client, FALSE);
        action_client_move(data, FALSE);
    }
    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_off(ObActionData *data, gpointer options)
{
    if (data->client) {
        action_client_move(data, TRUE);
        client_set_undecorated(data->client, TRUE);
        action_client_move(data, FALSE);
    }
    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_toggle(ObActionData *data, gpointer options)
{
    if (data->client) {
        action_client_move(data, TRUE);
        client_set_undecorated(data->client, !data->client->undecorated);
        action_client_move(data, FALSE);
    }
    return FALSE;
}
