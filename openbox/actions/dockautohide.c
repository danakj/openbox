#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/client_set.h"
#include "openbox/config.h"
#include "openbox/dock.h"

static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);

void action_dockautohide_startup(void)
{
    action_register("ToggleDockAutoHide", OB_ACTION_DEFAULT_FILTER_EMPTY,
                    NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options)
{
    config_dock_hide = !config_dock_hide;
    action_client_move(data, TRUE);
    dock_configure();
    action_client_move(data, FALSE);

    return FALSE;
}
