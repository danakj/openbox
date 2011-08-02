#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/client.h"

static gboolean run_func_toggle(const ObActionListRun *data, gpointer options);

void action_fullscreen_startup(void)
{
    action_register("ToggleFullscreen", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func_toggle);
}

/* Always return FALSE because its not interactive */
static gboolean run_func_toggle(const ObActionListRun *data, gpointer options)
{
    if (data->target) {
        action_client_move(data, TRUE);
        client_fullscreen(data->target, !data->target->fullscreen);
        action_client_move(data, FALSE);
    }
    return FALSE;
}
