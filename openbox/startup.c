#include "prop.h"
#include "screen.h"
#include "client.h"
#include "focus.h"
#include "config.h"
#include "openbox.h"

guint32 *startup_stack_order = NULL;
guint    startup_stack_size = 0;
guint32  startup_active = None;
guint32  startup_desktop = 0;

void startup_save()
{
    /* save the stacking order on startup! */
    PROP_GETA32(ob_root, net_client_list_stacking, window,
                (guint32**)&startup_stack_order, &startup_stack_size);
    PROP_GET32(ob_root, net_active_window, window, &startup_active);
    PROP_GET32(ob_root, net_current_desktop, cardinal, &startup_desktop);
}
