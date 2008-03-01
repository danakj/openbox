#include "hooks.h"
#include "actions.h"

#include <glib.h>

static GSList *hooks[OB_NUM_HOOKS*2];

void hooks_startup(gboolean reconfig)
{
}

void hooks_shutdown(gboolean reconfig)
{
    gint i;

    for (i = 0; i < OB_NUM_HOOKS*2; ++i)
        while (hooks[i]) {
            actions_act_unref(hooks[i]->data);
            hooks[i] = g_slist_delete_link(hooks[i], hooks[i]);
        }
}

ObHook hooks_hook_from_name(const gchar *n)
{
    if (!g_ascii_strcasecmp(n, "WindowNew"))
        return OB_HOOK_WIN_NEW;
    if (!g_ascii_strcasecmp(n, "WindowClosed"))
        return OB_HOOK_WIN_CLOSE;
    if (!g_ascii_strcasecmp(n, "WindowVisible"))
        return OB_HOOK_WIN_VISIBLE;
    if (!g_ascii_strcasecmp(n, "WindowInvisible"))
        return OB_HOOK_WIN_INVISIBLE;
    if (!g_ascii_strcasecmp(n, "WindowIconified"))
        return OB_HOOK_WIN_ICONIC;
    if (!g_ascii_strcasecmp(n, "WindowUniconified"))
        return OB_HOOK_WIN_UNICONIC;
    if (!g_ascii_strcasecmp(n, "WindowMaximized"))
        return OB_HOOK_WIN_MAX;
    if (!g_ascii_strcasecmp(n, "WindowUnmaximized"))
        return OB_HOOK_WIN_UNMAX;
    if (!g_ascii_strcasecmp(n, "WindowShaded"))
        return OB_HOOK_WIN_SHADE;
    if (!g_ascii_strcasecmp(n, "WindowUnshaded"))
        return OB_HOOK_WIN_UNSHADE;
    if (!g_ascii_strcasecmp(n, "WindowFocused"))
        return OB_HOOK_WIN_FOCUS;
    if (!g_ascii_strcasecmp(n, "WindowUnfocused"))
        return OB_HOOK_WIN_UNFOCUS;
    if (!g_ascii_strcasecmp(n, "WindowOnCurrentDesktop"))
        return OB_HOOK_WIN_CURRENT_DESK;
    if (!g_ascii_strcasecmp(n, "WindowOnOtherDesktop"))
        return OB_HOOK_WIN_OTHER_DESK;
    if (!g_ascii_strcasecmp(n, "WindowDecorated"))
        return OB_HOOK_WIN_DECORATED;
    if (!g_ascii_strcasecmp(n, "WindowUndecorated"))
        return OB_HOOK_WIN_UNDECORATED;
    return OB_HOOK_INVALID;
}

void hooks_fire(ObHook hook, struct _ObClient *c)
{
    GSList *it;

    g_assert(hook < OB_NUM_HOOKS);

    for (it = hooks[hook]; it; it = g_slist_next(it))
        actions_run_acts(it->data,
                         OB_USER_ACTION_HOOK,
                         0, -1, -1, 0,
                         OB_FRAME_CONTEXT_NONE,
                         c);
}

void hooks_add(ObHook hook, struct _ObActionsAct *act)
{
    g_assert(hook < OB_NUM_HOOKS);

    /* append so they are executed in the same order as they appear in the
       config file */
    hooks[hook] = g_slist_append(hooks[hook], act);
}
