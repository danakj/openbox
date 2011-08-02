#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/client.h"
#include "openbox/screen.h"

static gboolean run_func_toggle(const ObActionListRun *data, gpointer options);

void action_omnipresent_startup(void)
{
    action_register("ToggleOmnipresent", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func_toggle);
}

/* Always return FALSE because its not interactive */
static gboolean run_func_toggle(const ObActionListRun *data, gpointer options)
{
    if (data->target) {
        action_client_move(data, TRUE);
        client_set_desktop(data->target,
                           data->target->desktop == DESKTOP_ALL ?
                           screen_desktop : DESKTOP_ALL, FALSE, TRUE);
        action_client_move(data, FALSE);
    }
    return FALSE;
}
