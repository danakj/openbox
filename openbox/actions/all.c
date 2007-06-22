#include "all.h"

void action_all_startup()
{
    action_execute_startup();
    action_debug_startup();
    action_showmenu_startup();
    action_showdesktop_startup();
    action_reconfigure_startup();
    action_exit_startup();
}
