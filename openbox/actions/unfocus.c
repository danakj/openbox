#include "openbox/action.h"
#include "openbox/focus.h"

static gboolean run_func(ObActionData *data, gpointer options);

void action_unfocus_startup(void)
{
    action_register("Unfocus", NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionData *data, gpointer options)
{
    if (data->client && data->client == focus_client)
        focus_fallback(FALSE, FALSE, TRUE, FALSE);
    return FALSE;
}
