#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/client.h"
#include "openbox/client_set.h"
#include "openbox/moveresize.h"
#include "obt/prop.h"

static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);

void action_move_startup(void)
{
    action_register("Move", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options)
{
    GList *list;
    guint32 corner;
    ObClient *c;

    /* XXX only works on sets of size 1 right now until moveresize changes */
    if (client_set_size(set) != 1) return FALSE;

    list = client_set_get_all(set);
    c = list->data;
    g_list_free(list);

    corner = data->pointer_button != 0 ?
        OBT_PROP_ATOM(NET_WM_MOVERESIZE_MOVE) :
        OBT_PROP_ATOM(NET_WM_MOVERESIZE_MOVE_KEYBOARD);

    moveresize_start(c, data->pointer_x, data->pointer_y,
                     data->pointer_button, corner);
    return FALSE;
}
