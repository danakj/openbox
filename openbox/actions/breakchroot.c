#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/keyboard.h"

static gboolean run_func(const ObActionListRun *data, gpointer options);

void action_breakchroot_startup(void)
{
    action_register("BreakChroot",
                    OB_ACTION_DEFAULT_FILTER_EMPTY,
                    NULL, NULL,
                    run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObActionListRun *data, gpointer options)
{
    /* break out of one chroot */
    keyboard_reset_chains(1);

    return FALSE;
}
