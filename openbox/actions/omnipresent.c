#include "openbox/action.h"
#include "openbox/client.h"
#include "openbox/screen.h"

static gboolean run_func_toggle(ObActionData *data, gpointer options);

void action_omnipresent_startup(void)
{
    action_register("ToggleOmnipresent", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func_toggle);
}

/* Always return FALSE because its not interactive */
static gboolean run_func_toggle(ObActionData *data, gpointer options)
{
    if (data->client) {
        action_client_move(data, TRUE);
        client_set_desktop(data->client,
                           data->client->desktop == DESKTOP_ALL ?
                           screen_desktop : DESKTOP_ALL, FALSE, TRUE);
        action_client_move(data, FALSE);
    }
    return FALSE;
}
