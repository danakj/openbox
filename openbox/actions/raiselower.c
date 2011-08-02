#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/stacking.h"

static gboolean run_func(const ObActionListRun *data, gpointer options);

void action_raiselower_startup(void)
{
    action_register("RaiseLower", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObActionListRun *data, gpointer options)
{
    if (data->target) {
        action_client_move(data, TRUE);
        stacking_restack_request(data->target, NULL, Opposite);
        action_client_move(data, FALSE);
    }

    return FALSE;
}
