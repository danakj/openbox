#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/client.h"

static gboolean run_func(const ObActionListRun *data, gpointer options);

void action_kill_startup(void)
{
    action_register("Kill", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObActionListRun *data, gpointer options)
{
    if (data->target)
        client_kill(data->target);

    return FALSE;
}
