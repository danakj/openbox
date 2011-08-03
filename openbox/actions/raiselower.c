#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/client_set.h"
#include "openbox/stacking.h"

static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);

void action_raiselower_startup(void)
{
    action_register("RaiseLower", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func);
}

static gboolean each_run(struct _ObClient *c, const ObActionListRun *data,
                         gpointer options)
{
    stacking_restack_request(c, NULL, Opposite);
    return TRUE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options)
{
    if (!client_set_is_empty(set)) {
        action_client_move(data, TRUE);
        client_set_run(set, data, each_run, options);
        action_client_move(data, FALSE);
    }

    return FALSE;
}
