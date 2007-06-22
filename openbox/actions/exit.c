#include "openbox/actions.h"
#include "openbox/openbox.h"

static gboolean run_func(ObActionsData *data, gpointer options);

void action_exit_startup()
{
    actions_register("Exit",
                     NULL, NULL,
                     run_func,
                     NULL, NULL);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    ob_exit(0);

    return FALSE;
}
