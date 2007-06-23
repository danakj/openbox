#include "openbox/actions.h"
#include "openbox/screen.h"
#include "openbox/client.h"
#include "openbox/debug.h"
#include <glib.h>

typedef struct {
    gboolean current;
    gboolean add;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_add_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_remove_func(ObParseInst *i,
                                  xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_addremovedesktop_startup()
{
    actions_register("AddDesktop",
                     setup_add_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("RemoveDesktop",
                     setup_remove_func,
                     free_func,
                     run_func,
                     NULL, NULL);
}

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);

    if ((n = parse_find_node("where", node))) {
        gchar *s = parse_string(doc, n);
        if (!g_ascii_strcasecmp(s, "last"))
            o->current = FALSE;
        else if (!g_ascii_strcasecmp(s, "current"))
            o->current = TRUE;
        g_free(s);
    }

    return o;
}

static gpointer setup_add_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->add = TRUE;
    return o;
}

static gpointer setup_remove_func(ObParseInst *i,
                                  xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->add = FALSE;
    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;

    g_free(o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    actions_client_move(data, FALSE);

    if (o->add) {
        screen_set_num_desktops(screen_num_desktops+1);

        /* move all the clients over */
        if (o->current) {
            GList *it;

            for (it = client_list; it; it = g_list_next(it)) {
                ObClient *c = it->data;
                if (c->desktop != DESKTOP_ALL && c->desktop >= screen_desktop)
                    client_set_desktop(c, c->desktop+1, FALSE, TRUE);
            }
        }
    }
    else if (screen_num_desktops > 1) {
        guint rmdesktop, movedesktop;
        GList *it, *stacking_copy;

        /* what desktop are we removing and moving to? */
        if (o->current)
            rmdesktop = screen_desktop;
        else
            rmdesktop = screen_num_desktops - 1;
        if (rmdesktop < screen_num_desktops - 1)
            movedesktop = rmdesktop + 1;
        else
            movedesktop = rmdesktop;

        /* make a copy of the list cuz we're changing it */
        stacking_copy = g_list_copy(stacking_list);
        for (it = g_list_last(stacking_copy); it; it = g_list_previous(it)) {
            if (WINDOW_IS_CLIENT(it->data)) {
                ObClient *c = it->data;
                guint d = c->desktop;
                if (d != DESKTOP_ALL && d >= movedesktop) {
                    client_set_desktop(c, c->desktop - 1, TRUE, TRUE);
                    ob_debug("moving window %s\n", c->title);
                }
                /* raise all the windows that are on the current desktop which
                   is being merged */
                if ((screen_desktop == rmdesktop - 1 ||
                     screen_desktop == rmdesktop) &&
                    (d == DESKTOP_ALL || d == screen_desktop))
                {
                    stacking_raise(CLIENT_AS_WINDOW(c));
                    ob_debug("raising window %s\n", c->title);
                }
            }
        }

        /* act like we're changing desktops */
        if (screen_desktop < screen_num_desktops - 1) {
            gint d = screen_desktop;
            screen_desktop = screen_last_desktop;
            screen_set_desktop(d, TRUE);
            ob_debug("fake desktop change\n");
        }

        screen_set_num_desktops(screen_num_desktops-1);
    }

    actions_client_move(data, TRUE);

    return FALSE;
}
