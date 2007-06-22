#include "all.h"

void action_all_startup()
{
    action_execute_startup();
    action_debug_startup();
    action_showmenu_startup();
    action_showdesktop_startup();
    action_reconfigure_startup();
    action_exit_startup();
    action_restart_startup();
    action_cyclewindows_startup();
    action_activate_startup();
    action_breakchroot_startup();
    action_close_startup();
    action_move_startup();
}
