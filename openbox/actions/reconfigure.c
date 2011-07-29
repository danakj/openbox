#include "openbox/action.h"
#include "openbox/openbox.h"

static gboolean run_func(ObActionData *data, gpointer options);

void action_reconfigure_startup(void)
{
    action_register("Reconfigure", OB_ACTION_DEFAULT_FILTER_EMPTY,
                    NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionData *data, gpointer options)
{
    ob_reconfigure();

    return FALSE;
}
