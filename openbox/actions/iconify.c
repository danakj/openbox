#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/client.h"

static gboolean run_func(const ObActionListRun *data, gpointer options);

void action_iconify_startup(void)
{
    action_register("Iconify", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObActionListRun *data, gpointer options)
{
    if (data->target) {
        action_client_move(data, TRUE);
        client_iconify(data->target, TRUE, TRUE, FALSE);
        action_client_move(data, FALSE);
    }

    return FALSE;
}
