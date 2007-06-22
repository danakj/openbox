#include "openbox/actions.h"
#include "openbox/keyboard.h"

static gboolean run_func(ObActionsData *data, gpointer options);

void action_breakchroot_startup()
{
    actions_register("BreakChroot",
                     NULL, NULL,
                     run_func,
                     NULL, NULL);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    /* break out of one chroot */
    keyboard_reset_chains(1);

    return FALSE;
}
