#include "kernel/openbox.h"
#include "kernel/dispatch.h"
#include "kernel/frame.h"
#include "kernel/client.h"
#include "kernel/screen.h"
#include "parser/parse.h"
#include "history.h"
#include <glib.h>
#include <string.h>
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#define PLACED        (1 << 0)

#define HAVE_POSITION (1 << 1)
#define HAVE_SIZE     (1 << 2)
#define HAVE_DESKTOP  (1 << 3)

struct HistoryItem {
    char *name;
    char *class;
    char *role;

    int flags;

    int x, y;
    int w, h;
    guint desk;
};

static GSList *history_list = NULL;
static char *history_path = NULL;

static struct HistoryItem *history_find(const char *name, const char *class,
                                 const char *role)
{
    GSList *it;
    struct HistoryItem *hi = NULL;

    /* find the client */
    for (it = history_list; it != NULL; it = it->next) {
        hi = it->data;
        if (!strcmp(hi->name, name) &&
            !strcmp(hi->class, class) &&
            !strcmp(hi->role, role))
            return hi;
    }
    return NULL;
}

gboolean place_history(Client *c)
{
    struct HistoryItem *hi;
    int x, y, w, h;

    hi = history_find(c->name, c->class, c->role);

    if (hi && !(hi->flags & PLACED)) {
        hi->flags |= PLACED;
        if (ob_state != OB_STATE_STARTING) {
            if (hi->flags & HAVE_POSITION ||
                hi->flags & HAVE_SIZE) {
                if (hi->flags & HAVE_POSITION) {
                    x = hi->x;
                    y = hi->y;
                    /* get where the client should be */
                    frame_frame_gravity(c->frame, &x, &y);
                } else {
                    x = c->area.x;
                    y = c->area.y;
                }
                if (hi->flags & HAVE_SIZE) {
                    w = hi->w * c->size_inc.width;
                    h = hi->h * c->size_inc.height;
                } else {
                    w = c->area.width;
                    h = c->area.height;
                }
                client_configure(c, OB_CORNER_TOPLEFT, x, y, w, h,
                                 TRUE, TRUE);
            }
            if (hi->flags & HAVE_DESKTOP) {
                client_set_desktop(c, hi->desk, FALSE);
            }
        }
        return hi->flags & HAVE_POSITION;
    }

    return FALSE;
}

static void set_history(Client *c)
{
    struct HistoryItem *hi;

    hi = history_find(c->name, c->class, c->role);

    if (hi == NULL) {
        hi = g_new(struct HistoryItem, 1);
        history_list = g_slist_append(history_list, hi);
        hi->name = g_strdup(c->name);
        hi->class = g_strdup(c->class);
        hi->role = g_strdup(c->role);
        hi->flags = HAVE_POSITION;
    }

    if (hi->flags & HAVE_POSITION) {
        hi->x = c->frame->area.x;
        hi->y = c->frame->area.y;
    }

    hi->flags &= ~PLACED;
}

static void event(ObEvent *e, void *foo)
{
    g_assert(e->type == Event_Client_Destroy);

    set_history(e->data.c.client);
}

/*

<entry name="name" class="class" role="role">
  <x>0</x>
  <y>0</y>
  <width>300</width>
  <height>200</height>
  <desktop>1</desktop>
</entry>

*/

static void save_history()
{
    xmlDocPtr doc;
    xmlNodePtr root, node;
    char *s;
    GSList *it;

    doc = xmlNewDoc(NULL);
    root = xmlNewNode(NULL, (const xmlChar*) "openbox_history");
    xmlDocSetRootElement(doc, root);

    for (it = history_list; it; it = g_slist_next(it)) {
        struct HistoryItem *hi = it->data;
        g_message("adding %s", hi->name);
        node = xmlNewChild(root, NULL, (const xmlChar*) "entry", NULL);
        xmlNewProp(node, (const xmlChar*) "name", (const xmlChar*) hi->name);
        xmlNewProp(node, (const xmlChar*) "class", (const xmlChar*) hi->class);
        xmlNewProp(node, (const xmlChar*) "role", (const xmlChar*) hi->role);
        if (hi->flags & HAVE_POSITION) {
            s = g_strdup_printf("%d", hi->x);
            xmlNewTextChild(node, NULL,
                            (const xmlChar*) "x", (const xmlChar*) s);
            g_free(s);
            s = g_strdup_printf("%d", hi->y);
            xmlNewTextChild(node, NULL,
                            (const xmlChar*) "y", (const xmlChar*) s);
            g_free(s);
        }
        if (hi->flags & HAVE_SIZE) {
            s = g_strdup_printf("%d", hi->w);
            xmlNewTextChild(node, NULL,
                            (const xmlChar*) "width", (const xmlChar*) s);
            g_free(s);
            s = g_strdup_printf("%d", hi->h);
            xmlNewTextChild(node, NULL,
                            (const xmlChar*) "height", (const xmlChar*) s);
            g_free(s);
        }
        if (hi->flags & HAVE_DESKTOP) {
            s = g_strdup_printf("%d", hi->desk < 0 ? hi->desk : hi->desk + 1);
            xmlNewTextChild(node, NULL,
                            (const xmlChar*) "desktop", (const xmlChar*) s);
            g_free(s);
        }
    }

    xmlIndentTreeOutput = 1;
    xmlSaveFormatFile(history_path, doc, 1);

    xmlFree(doc);
}

static void load_history()
{
    xmlDocPtr doc;
    xmlNodePtr node, n;
    char *name;
    char *class;
    char *role;
    struct HistoryItem *hi;

    if (!parse_load(history_path, "openbox_history", &doc, &node))
        return;

    node = parse_find_node("entry", node->xmlChildrenNode);
    while (node) {
        name = class = role = NULL;
        if (parse_attr_string("name", node, &name) &&
            parse_attr_string("class", node, &class) &&
            parse_attr_string("role", node, &role)) {

            hi = history_find(name, class, role);
            if (!hi) {
                hi = g_new(struct HistoryItem, 1);
                hi->name = g_strdup(name);
                hi->class = g_strdup(class);
                hi->role = g_strdup(role);
                hi->flags = 0;
            }
            if ((n = parse_find_node("x", node->xmlChildrenNode))) {
                hi->x = parse_int(doc, n);
                if ((n = parse_find_node("y", node->xmlChildrenNode))) {
                    hi->y = parse_int(doc, n);
                    hi->flags |= HAVE_POSITION;
                }
            }
            if ((n = parse_find_node("width", node->xmlChildrenNode))) {
                hi->w = parse_int(doc, n);
                if ((n = parse_find_node("height", node->xmlChildrenNode))) {
                    hi->h = parse_int(doc, n);
                    hi->flags |= HAVE_SIZE;
                }
            }
            if ((n = parse_find_node("desktop", node->xmlChildrenNode))) {
                hi->desk = parse_int(doc, n);
                if (hi->desk > 0) --hi->desk;
                hi->flags |= HAVE_DESKTOP;
            }

            history_list = g_slist_append(history_list, hi);
        }
        g_free(name); g_free(class); g_free(role);
        node = parse_find_node("entry", node->next);
    }
    xmlFree(doc);
}

void history_startup()
{
    char *path;

    history_list = NULL;

    path = g_build_filename(g_get_home_dir(), ".openbox", "history", NULL);
    history_path = g_strdup_printf("%s.%d", path, ob_screen);
    g_free(path);

    load_history(); /* load from the historydb file */

    dispatch_register(Event_Client_Destroy, (EventHandler)event, NULL);
}

void history_shutdown()
{
    GSList *it;

    save_history(); /* save to the historydb file */
    for (it = history_list; it != NULL; it = it->next) {
        struct HistoryItem *hi = it->data;
        g_free(hi->name);
        g_free(hi->class);
        g_free(hi->role);
        g_free(hi);
    }
    g_slist_free(history_list);

    dispatch_register(0, (EventHandler)event, NULL);

    g_free(history_path);
}
