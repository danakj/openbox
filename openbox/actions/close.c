#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/client.h"

static gboolean run_func(const ObActionListRun *data, gpointer options);

void action_close_startup(void)
{
    action_register("Close", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObActionListRun *data, gpointer options)
{
    if (data->client) client_close(data->client);

    return FALSE;
}
