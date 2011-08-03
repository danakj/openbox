#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/client_set.h"
#include "openbox/openbox.h"

static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);

void action_reconfigure_startup(void)
{
    action_register("Reconfigure", OB_ACTION_DEFAULT_FILTER_EMPTY,
                    NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options)
{
    ob_reconfigure();

    return FALSE;
}
