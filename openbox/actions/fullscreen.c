#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/client.h"
#include "openbox/client_set.h"

static gboolean run_func_on(const ObClientSet *set,
                            const ObActionListRun *data, gpointer options);
static gboolean run_func_off(const ObClientSet *set,
                             const ObActionListRun *data, gpointer options);
static gboolean run_func_toggle(const ObClientSet *set,
                                const ObActionListRun *data, gpointer options);

void action_fullscreen_startup(void)
{
    action_register("Fullscreen", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func_on);
    action_register("Unfullscreen", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func_off);
    action_register("ToggleFullscreen", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func_toggle);
}

static gboolean each_on(ObClient *c, const ObActionListRun *data,
                         gpointer options)
{
    client_fullscreen(c, TRUE);
    return TRUE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_on(const ObClientSet *set,
                            const ObActionListRun *data, gpointer options)
{
    action_client_move(data, TRUE);
    client_set_run(set, data, each_on, options);
    action_client_move(data, FALSE);
    return FALSE;
}

static gboolean each_off(ObClient *c, const ObActionListRun *data,
                         gpointer options)
{
    client_fullscreen(c, FALSE);
    return TRUE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_off(const ObClientSet *set,
                             const ObActionListRun *data, gpointer options)
{
    action_client_move(data, TRUE);
    client_set_run(set, data, each_off, options);
    action_client_move(data, FALSE);
    return FALSE;
}

static gboolean each_toggle(ObClient *c, const ObActionListRun *data,
                         gpointer options)
{
    client_fullscreen(c, !c->fullscreen);
    return TRUE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_toggle(const ObClientSet *set,
                                const ObActionListRun *data, gpointer options)
{
    if (!client_set_is_empty(set)) {
        action_client_move(data, TRUE);
        client_set_run(set, data, each_toggle, options);
        action_client_move(data, FALSE);
    }
    return FALSE;
}
