#include "openbox/actions.h"
#include "openbox/screen.h"
#include <glib.h>

typedef struct {
    /* index is 0-indexed */
    guint index;
    gboolean current;
    gboolean last;
    gboolean add;
    gboolean follow;
} Options;

static gpointer setup_func(xmlNodePtr node, gboolean add);
static gpointer setup_add_func(xmlNodePtr node);
static gpointer setup_remove_func(xmlNodePtr node);
static void free_func(gpointer o);
static gboolean run_func(ObActionsData *data, gpointer options);
/* 3.4-compatibility */
static gpointer setup_addcurrent_func(xmlNodePtr node);
static gpointer setup_addlast_func(xmlNodePtr node);
static gpointer setup_removecurrent_func(xmlNodePtr node);
static gpointer setup_removelast_func(xmlNodePtr node);

void action_addremovedesktop_startup(void)
{
    actions_register("AddDesktop", setup_add_func, free_func, run_func);
    actions_register("RemoveDesktop", setup_remove_func, free_func, run_func);

    /* 3.4-compatibility */
    actions_register("AddDesktopLast", setup_addlast_func,
                     free_func, run_func);
    actions_register("RemoveDesktopLast", setup_removelast_func,
                     free_func, run_func);
    actions_register("AddDesktopCurrent", setup_addcurrent_func,
                     free_func, run_func);
    actions_register("RemoveDesktopCurrent", setup_removecurrent_func,
                     free_func, run_func);
}

static gpointer setup_func(xmlNodePtr node, gboolean add)
{
    xmlNodePtr n;
    Options *o;
    gchar *s;

    o = g_slice_new0(Options);
    s = obt_xml_node_string(node);

    o->add = add;

    if ((n = obt_xml_find_node(node, "where"))) {
        gchar *s = obt_xml_node_string(n);

        if (o->add) {
            o->follow = FALSE;
            if (!g_ascii_strcasecmp(s, "last")) {
                /* set value of index one greater than index of last desktop
                   and convert from 1-index to 0-index */
                o->last = TRUE;
            }
            else if (!g_ascii_strcasecmp(s, "current")) {
                /* screen_desktop is 0-indexed */
                o->current = TRUE;
                o->follow = TRUE;

            }
            else if (!g_ascii_strcasecmp(s, "first")) {
                o->index = 0;
            }
            else if (atoi(s) >= 0) {
                /* configuration input is 1-indexed */
                o->index = atoi(s) - 1;
            }
        }
        else {
            if (!g_ascii_strcasecmp(s, "last")) {
                /* set value of index to index of last desktop
                   and convert from 1-index to 0-index */
                o->last = TRUE;
            }
            else if (!g_ascii_strcasecmp(s, "current")) {
                /* screen_desktop is 0-indexed */
                o->current = TRUE;
            }
            else if (!g_ascii_strcasecmp(s, "first")) {
                o->index = 0;
            }
            else if (atoi(s) >= 0) {
                /* configuration input is 1-indexed */
                o->index = atoi(s) - 1;
            }
        }
        ob_debug("Desktop add/remove index: %d", o->index);
        g_free(s);
    }

    return o;
}

static gpointer setup_add_func(xmlNodePtr node)
{
    Options *o = setup_func(node, TRUE);
    return o;
}

static gpointer setup_remove_func(xmlNodePtr node)
{
    Options *o = setup_func(node, FALSE);
    return o;
}

static void free_func(gpointer o)
{
    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    actions_client_move(data, TRUE);

    if (o->add){
        if (o->current) {
            o->index = screen_desktop;
        }
        else if (o->last) {
            o->index = screen_num_desktops;
        }
        screen_add_desktop(o->index, o->follow);
    } else {
        if (o->current) {
            o->index = screen_desktop;
        }
        else if (o->last) {
            o->index = screen_num_desktops - 1;
        }
        screen_remove_desktop(o->index);
    }

    actions_client_move(data, FALSE);

    return FALSE;
}

/* 3.4-compatibility */
static gpointer setup_addcurrent_func(xmlNodePtr node)
{
    Options *o = setup_add_func(node);
    o->index = screen_desktop;
    o->follow = TRUE;
    return o;
}

static gpointer setup_addlast_func(xmlNodePtr node)
{
    Options *o = setup_add_func(node);
    o->index = screen_num_desktops;
    return o;
}

static gpointer setup_removecurrent_func(xmlNodePtr node)
{
    Options *o = setup_remove_func(node);
    o->index = screen_desktop;
    return o;
}

static gpointer setup_removelast_func(xmlNodePtr node)
{
    Options *o = setup_remove_func(node);
    o->index = screen_num_desktops - 1;
    return o;
}
