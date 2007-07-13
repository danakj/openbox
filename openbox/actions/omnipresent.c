#include "openbox/actions.h"
#include "openbox/client.h"
#include "openbox/screen.h"

static gboolean run_func_toggle(ObActionsData *data, gpointer options);

void action_omnipresent_startup()
{
    actions_register("ToggleOmnipresent", NULL, NULL, run_func_toggle,
                     NULL, NULL);
}

/* Always return FALSE because its not interactive */
static gboolean run_func_toggle(ObActionsData *data, gpointer options)
{
    if (data->client)
        client_set_desktop(data->client,
                           data->client->desktop == DESKTOP_ALL ?
                           screen_desktop : DESKTOP_ALL, FALSE, TRUE);
    return FALSE;
}
