#include "openbox/actions.h"
#include "openbox/openbox.h"

static gboolean run_func(ObActionsData *data, gpointer options);

void action_reconfigure_startup()
{
    actions_register("Reconfigure",
                     NULL, NULL,
                     run_func,
                     NULL, NULL);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    ob_reconfigure();

    return FALSE;
}
