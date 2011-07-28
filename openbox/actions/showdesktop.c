#include "openbox/action.h"
#include "openbox/screen.h"

static gboolean run_func(ObActionData *data, gpointer options);

void action_showdesktop_startup(void)
{
    action_register("ToggleShowDesktop", NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionData *data, gpointer options)
{
    screen_show_desktop(!screen_showing_desktop, NULL);

    return FALSE;
}
