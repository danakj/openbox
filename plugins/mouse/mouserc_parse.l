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

    a = action_from_string(action);
 
    if (mact == MouseAction_Motion) {
        if (a && !(a->func == action_move || a->func == action_resize)) {
            action_free(a);
            a = NULL;
        }
        /* the below types cannot be used with !motion events, or at all with
           mouse bindings */
    } else if (a && (a->func == action_move || a->func == action_resize ||
                     a->func == action_desktop ||
                     a->func == action_move_relative ||
                     a->func == action_resize_relative)) {
        action_free(a);
        a = NULL;
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
