#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/client.h"

static gboolean run_func_on(const ObActionListRun *data, gpointer options);
static gboolean run_func_off(const ObActionListRun *data, gpointer options);
static gboolean run_func_toggle(const ObActionListRun *data, gpointer options);

void action_shade_startup(void)
{
    action_register("Shade", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func_on);
    action_register("Unshade", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func_off);
    action_register("ToggleShade", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func_toggle);
}

/* Always return FALSE because its not interactive */
static gboolean run_func_on(const ObActionListRun *data, gpointer options)
{
    if (data->target) {
        action_client_move(data, TRUE);
        client_shade(data->target, TRUE);
        action_client_move(data, FALSE);
    }
    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_off(const ObActionListRun *data, gpointer options)
{
    if (data->target) {
        action_client_move(data, TRUE);
        client_shade(data->target, FALSE);
        action_client_move(data, FALSE);
    }
    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_toggle(const ObActionListRun *data, gpointer options)
{
    if (data->target) {
        action_client_move(data, TRUE);
        client_shade(data->target, !data->target->shaded);
        action_client_move(data, FALSE);
    }
    return FALSE;
}
