#include "openbox/actions.h"
#include "openbox/focus.h"

static gboolean run_func(ObActionsData *data, gpointer options);

void action_unfocus_startup()
{
    actions_register("Unfocus", NULL, NULL, run_func, NULL, NULL);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    if (data->client && data->client == focus_client)
        focus_fallback(FALSE, FALSE, TRUE);
    return FALSE;
}