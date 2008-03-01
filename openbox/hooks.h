#ifndef ob__hooks_h
#define ob__hooks_h

#include <glib.h>

struct _ObActionsAct;
struct _ObClient;

typedef enum {
    OB_HOOK_INVALID,
    OB_HOOK_WIN_NEW,
    OB_HOOK_WIN_CLOSE,
    OB_HOOK_WIN_VISIBLE,
    OB_HOOK_WIN_INVISIBLE,
    OB_HOOK_WIN_ICONIC,
    OB_HOOK_WIN_UNICONIC,
    OB_HOOK_WIN_MAX,
    OB_HOOK_WIN_UNMAX,
    OB_HOOK_WIN_SHADE,
    OB_HOOK_WIN_UNSHADE,
    OB_HOOK_WIN_FOCUS,
    OB_HOOK_WIN_UNFOCUS,
    OB_HOOK_WIN_DESK_CHANGE,
    OB_HOOK_WIN_DECORATED,
    OB_HOOK_WIN_UNDECORATED,
    OB_HOOK_SCREEN_DESK_CHANGE,
    OB_NUM_HOOKS
} ObHook;

void hooks_startup(gboolean reconfig);
void hooks_shutdown(gboolean reconfig);

ObHook hooks_hook_from_name(const gchar *n);

/*! Run a hook.
  @param on TRUE if the hook is being run cuz a state was turned on, FALSE
            if a state was turned off
*/
void hooks_run(ObHook hook, struct _ObClient *c);

void hooks_add(ObHook hook, struct _ObActionsAct *act);

#endif
