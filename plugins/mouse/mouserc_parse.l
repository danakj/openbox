%{
#include "mouse.h"
#include <glib.h>
#ifdef HAVE_STDIO_H
#  include <stdio.h>
#endif

static int lineno;
static char *path;
static gboolean error;

static char *context;
static char *event;
static char *button;
static char *action;

static void endofline();
static int mparsewrap();
static void gotfield();
static void addbinding();
%}

field [A-Za-z0-9][-A-Za-z0-9]*
white [ \t]*

%%

^{white}#.*\n lineno++;
{field} gotfield();
\n endofline();
[ \t]
. error = TRUE;

%%

static void gotfield()
{
    if (context == NULL)
        context = g_strdup(mparsetext);
    else if (event == NULL)
        event = g_strdup(mparsetext);
    else if (button == NULL)
        button = g_strdup(mparsetext);
    else if (action == NULL)
        action = g_strdup(mparsetext);
    else
        error = TRUE;
}

static void endofline()
{
    if (!error && context && event && button && action)
        addbinding();
    else if (error || context || event || button || action)
        g_warning("Parser error in '%s' on line %d", path, lineno);

    error = FALSE;
    g_free(context); g_free(event); g_free(button); g_free(action);
    context = event = button = action = NULL;

    ++lineno;
}

static void addbinding()
{
    Action *a = NULL;
    MouseAction mact;

    if (!g_ascii_strcasecmp(event, "press"))
        mact = MouseAction_Press;
    else if (!g_ascii_strcasecmp(event, "release"))
        mact = MouseAction_Release;
    else if (!g_ascii_strcasecmp(event, "click"))
        mact = MouseAction_Click;
    else if (!g_ascii_strcasecmp(event, "doubleclick"))
        mact = MouseAction_DClick;
    else if (!g_ascii_strcasecmp(event, "drag"))
        mact = MouseAction_Motion;
    else {
        g_warning("Invalid event '%s' in '%s' on line %d", event, path,
                  lineno);
        return;
    }

    if (mact == MouseAction_Motion) {
        if (!g_ascii_strcasecmp(action, "move") &&
            mact == MouseAction_Motion) {
            a = action_new(action_move);
        } else if (!g_ascii_strcasecmp(action, "resize") &&
                   mact == MouseAction_Motion) {
            a = action_new(action_resize);
        }
    } else {
        if (!g_ascii_strcasecmp(action, "focus")) {
            a = action_new(action_focus);
        } else if (!g_ascii_strcasecmp(action, "unfocus")) {
            a = action_new(action_unfocus);
        } else if (!g_ascii_strcasecmp(action, "iconify")) {
            a = action_new(action_iconify);
        } else if (!g_ascii_strcasecmp(action, "raise")) {
            a = action_new(action_raise);
        } else if (!g_ascii_strcasecmp(action, "lower")) {
            a = action_new(action_lower);
        } else if (!g_ascii_strcasecmp(action, "focusraise")) {
            a = action_new(action_focusraise);
        } else if (!g_ascii_strcasecmp(action, "close")) {
            a = action_new(action_close);
        } else if (!g_ascii_strcasecmp(action, "kill")) {
            a = action_new(action_kill);
        } else if (!g_ascii_strcasecmp(action, "shade")) {
            a = action_new(action_shade);
        } else if (!g_ascii_strcasecmp(action, "unshade")) {
            a = action_new(action_unshade);
        } else if (!g_ascii_strcasecmp(action, "toggleshade")) {
            a = action_new(action_toggle_shade);
        } else if (!g_ascii_strcasecmp(action, "toggleomnipresent")) {
            a = action_new(action_toggle_omnipresent);
        } else if (!g_ascii_strcasecmp(action, "maximizefull")) {
            a = action_new(action_maximize_full);
        } else if (!g_ascii_strcasecmp(action, "unmaximizefull")) {
            a = action_new(action_unmaximize_full);
        } else if (!g_ascii_strcasecmp(action, "togglemaximizefull")) {
            a = action_new(action_toggle_maximize_full);
        } else if (!g_ascii_strcasecmp(action, "maximizehorz")) {
            a = action_new(action_maximize_horz);
        } else if (!g_ascii_strcasecmp(action, "unmaximizehorz")) {
            a = action_new(action_unmaximize_horz);
        } else if (!g_ascii_strcasecmp(action, "togglemaximizehorz")) {
            a = action_new(action_toggle_maximize_horz);
        } else if (!g_ascii_strcasecmp(action, "maximizevert")) {
            a = action_new(action_maximize_vert);
        } else if (!g_ascii_strcasecmp(action, "unmaximizevert")) {
            a = action_new(action_unmaximize_vert);
        } else if (!g_ascii_strcasecmp(action, "togglemaximizevert")) {
            a = action_new(action_toggle_maximize_vert);
        } else if (!g_ascii_strcasecmp(action, "sendtonextdesktop")) {
            a = action_new(action_send_to_next_desktop);
            a->data.sendtonextprev.wrap = FALSE;
            a->data.sendtonextprev.follow = TRUE;
        } else if (!g_ascii_strcasecmp(action, "sendtonextdesktopwrap")) {
            a = action_new(action_send_to_next_desktop);
            a->data.sendtonextprev.wrap = TRUE;
            a->data.sendtonextprev.follow = TRUE;
        } else if (!g_ascii_strcasecmp(action, "sendtopreviousdesktop")) {
            a = action_new(action_send_to_previous_desktop);
            a->data.sendtonextprev.wrap = FALSE;
            a->data.sendtonextprev.follow = TRUE;
        } else if (!g_ascii_strcasecmp(action, "sendtopreviousdesktopwrap")) {
            a = action_new(action_send_to_previous_desktop);
            a->data.sendtonextprev.wrap = TRUE;
            a->data.sendtonextprev.follow = TRUE;
        } else if (!g_ascii_strcasecmp(action, "nextdesktop")) {
            a = action_new(action_next_desktop);
            a->data.nextprevdesktop.wrap = FALSE;
        } else if (!g_ascii_strcasecmp(action, "nextdesktopwrap")) {
            a = action_new(action_next_desktop);
            a->data.nextprevdesktop.wrap = TRUE;
        } else if (!g_ascii_strcasecmp(action, "previousdesktop")) {
            a = action_new(action_previous_desktop);
            a->data.nextprevdesktop.wrap = FALSE;
        } else if (!g_ascii_strcasecmp(action, "previousdesktopwrap")) {
            a = action_new(action_previous_desktop);
            a->data.nextprevdesktop.wrap = TRUE;
        } else if (!g_ascii_strcasecmp(action, "nextdesktopcolumn")) {
            a = action_new(action_next_desktop_column);
            a->data.nextprevdesktop.wrap = FALSE;
        } else if (!g_ascii_strcasecmp(action, "nextdesktopcolumnwrap")) {
            a = action_new(action_next_desktop_column);
            a->data.nextprevdesktop.wrap = TRUE;
        } else if (!g_ascii_strcasecmp(action, "previousdesktopcolumn")) {
            a = action_new(action_previous_desktop_column);
            a->data.nextprevdesktop.wrap = FALSE;
        } else if (!g_ascii_strcasecmp(action, "previousdesktopcolumnwrap")) {
            a = action_new(action_previous_desktop_column);
            a->data.nextprevdesktop.wrap = TRUE;
        } else if (!g_ascii_strcasecmp(action, "nextdesktoprow")) {
            a = action_new(action_next_desktop_row);
            a->data.nextprevdesktop.wrap = FALSE;
        } else if (!g_ascii_strcasecmp(action, "nextdesktoprowwrap")) {
            a = action_new(action_next_desktop_row);
            a->data.nextprevdesktop.wrap = TRUE;
        } else if (!g_ascii_strcasecmp(action, "previousdesktoprow")) {
            a = action_new(action_previous_desktop_row);
            a->data.nextprevdesktop.wrap = FALSE;
        } else if (!g_ascii_strcasecmp(action, "previousdesktoprowwrap")) {
            a = action_new(action_previous_desktop_row);
            a->data.nextprevdesktop.wrap = TRUE;
        }
    }
    if (a == NULL) {
        g_warning("Invalid action '%s' in '%s' on line %d", action, path,
                  lineno);
        return;
    }

    if (!mbind(button, context, mact, a)) {
        action_free(a);
        g_warning("Unable to add binding '%s %s %s %s'",
                  context, event, button, action);
    }
}


static int mparsewrap()
{
    g_free(context); g_free(event); g_free(button); g_free(action);
    return 1;
}

void mouserc_parse()
{
    path = g_build_filename(g_get_home_dir(), ".openbox", "mouserc", NULL);
    if ((mparsein = fopen(path, "r")) == NULL) {
        g_free(path);
        path = g_build_filename(RCDIR, "mouserc", NULL);
        if ((mparsein = fopen(path, "r")) == NULL) {
            g_warning("No mouserc file found!");
            g_free(path);
            return;
        }
    }

    lineno = 1;
    error = FALSE;
    context = event = button = action = NULL;

    mparselex();

    g_free(path);
}
