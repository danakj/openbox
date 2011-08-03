#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/client_set.h"
#include "openbox/stacking.h"
#include "openbox/window.h"
#include "openbox/dock.h"

static gboolean raise_func(const ObClientSet *set,
                           const ObActionListRun *data, gpointer options);
static gboolean lower_func(const ObClientSet *set,
                           const ObActionListRun *data, gpointer options);

void action_dock_startup(void)
{
    action_register("RaiseDock", OB_ACTION_DEFAULT_FILTER_EMPTY,
                    NULL, NULL, raise_func);
    action_register("LowerDock", OB_ACTION_DEFAULT_FILTER_EMPTY,
                    NULL, NULL, lower_func);
}

/* Always return FALSE because its not interactive */
static gboolean raise_func(const ObClientSet *set,
                           const ObActionListRun *data, gpointer options)
{
    action_client_move(data, TRUE);
    dock_raise_dock();
    action_client_move(data, FALSE);

    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean lower_func(const ObClientSet *set,
                           const ObActionListRun *data, gpointer options)
{
    action_client_move(data, TRUE);
    dock_lower_dock();
    action_client_move(data, FALSE);

    return FALSE;
}

