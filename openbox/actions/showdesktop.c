#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/screen.h"

static gboolean run_func(const ObActionListRun *data, gpointer options);

void action_showdesktop_startup(void)
{
    action_register("ToggleShowDesktop", OB_ACTION_DEFAULT_FILTER_EMPTY,
                    NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObActionListRun *data, gpointer options)
{
    screen_show_desktop(!screen_showing_desktop, NULL);

    return FALSE;
}
