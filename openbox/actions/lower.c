#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/stacking.h"
#include "openbox/window.h"

static gboolean run_func(const ObActionListRun *data, gpointer options);

void action_lower_startup(void)
{
    action_register("Lower", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObActionListRun *data, gpointer options)
{
    if (data->client) {
        action_client_move(data, TRUE);
        stacking_lower(CLIENT_AS_WINDOW(data->client));
        action_client_move(data, FALSE);
    }

    return FALSE;
}
