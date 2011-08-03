#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/client.h"
#include "openbox/client_set.h"

static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);

void action_iconify_startup(void)
{
    action_register("Iconify", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func);
}

static gboolean each_run(ObClient *c, const ObActionListRun *data,
                         gpointer options)
{
    client_iconify(c, TRUE, TRUE, FALSE);
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
