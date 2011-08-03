#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/client_set.h"
#include "openbox/focus.h"

static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);

void action_focustobottom_startup(void)
{
    action_register("FocusToBottom", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func);
}

static gboolean each_run(struct _ObClient *c, const ObActionListRun *data,
                         gpointer options)
{
    focus_order_to_bottom(c);
    return TRUE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options)
{
    client_set_run(set, data, each_run, options);
    return FALSE;
}
