#include "openbox/action.h"
#include "openbox/stacking.h"
#include "openbox/window.h"
#include "openbox/dock.h"

static gboolean raise_func(ObActionData *data, gpointer options);
static gboolean lower_func(ObActionData *data, gpointer options);

void action_dock_startup(void)
{
    action_register("RaiseDock", OB_ACTION_DEFAULT_FILTER_EMPTY,
                    NULL, NULL, raise_func);
    action_register("LowerDock", OB_ACTION_DEFAULT_FILTER_EMPTY,
                    NULL, NULL, lower_func);
}

/* Always return FALSE because its not interactive */
static gboolean raise_func(ObActionData *data, gpointer options)
{
    action_client_move(data, TRUE);
    dock_raise_dock();
    action_client_move(data, FALSE);

    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean lower_func(ObActionData *data, gpointer options)
{
    action_client_move(data, TRUE);
    dock_lower_dock();
    action_client_move(data, FALSE);

    return FALSE;
}

