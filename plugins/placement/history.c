#include "kernel/openbox.h"
#include "kernel/dispatch.h"
#include "kernel/frame.h"
#include "kernel/client.h"
#include <glib.h>
#include <string.h>
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

struct HistoryItem {
    char *name;
    char *class;
    char *role;
    int x;
    int y;
    gboolean placed;
};

static GSList *history = NULL;
static char *history_path = NULL;

static struct HistoryItem *find_history(Client *c)
{
    GSList *it;
    struct HistoryItem *hi = NULL;

    /* find the client */
    for (it = history; it != NULL; it = it->next) {
        hi = it->data;
        g_assert(hi->name != NULL);
        g_assert(hi->class != NULL);
        g_assert(hi->role != NULL);
        g_assert(c->name != NULL);
        g_assert(c->class != NULL);
        g_assert(c->role != NULL);
        if (!strcmp(hi->name, c->name) &&
            !strcmp(hi->class, c->class) &&
            !strcmp(hi->role, c->role))
            return hi;
    }
    return NULL;
}

gboolean place_history(Client *c)
{
    struct HistoryItem *hi;
    int x, y;

    hi = find_history(c);

    if (hi != NULL && !hi->placed) {
        hi->placed = TRUE;
        if (ob_state != State_Starting) {
            x = hi->x;
            y = hi->y;

            frame_frame_gravity(c->frame, &x, &y); /* get where the client
                                                      should be */
            client_configure(c, Corner_TopLeft, x, y,
                             c->area.width, c->area.height,
                             TRUE, TRUE);
        }
        return TRUE;
    }

    return FALSE;
}

static void strip_tabs(char *s)
{
    while (*s != '\0') {
        if (*s == '\t')
            *s = ' ';
        ++s;
    }
}

static void set_history(Client *c)
{
    struct HistoryItem *hi;

    hi = find_history(c);

    if (hi == NULL) {
        hi = g_new(struct HistoryItem, 1);
        history = g_slist_append(history, hi);
        hi->name = g_strdup(c->name);
        strip_tabs(hi->name);
        hi->class = g_strdup(c->class);
        strip_tabs(hi->class);
        hi->role = g_strdup(c->role);
        strip_tabs(hi->role);
    }

    hi->x = c->frame->area.x;
    hi->y = c->frame->area.y;
    hi->placed = FALSE;
}

static void event(ObEvent *e, void *foo)
{
    g_assert(e->type == Event_Client_Destroy);

    set_history(e->data.c.client);
}

static void save_history()
{
    GError *err = NULL;
    GIOChannel *io;
    GString *buf;
    GSList *it;
    struct HistoryItem *hi;
    gsize ret;

    io = g_io_channel_new_file(history_path, "w", &err);
    if (io != NULL) {
        for (it = history; it != NULL; it = it->next) {
            hi = it->data;
            buf = g_string_sized_new(0);
            buf=g_string_append(buf, hi->name);
            g_string_append_c(buf, '\t');
            buf=g_string_append(buf, hi->class);
            g_string_append_c(buf, '\t');
            buf=g_string_append(buf, hi->role);
            g_string_append_c(buf, '\t');
            g_string_append_printf(buf, "%d", hi->x);
            buf=g_string_append_c(buf, '\t');
            g_string_append_printf(buf, "%d", hi->y);
            buf=g_string_append_c(buf, '\n');
            if (g_io_channel_write_chars(io, buf->str, buf->len, &ret, &err) !=
                G_IO_STATUS_NORMAL)
                break;
            g_string_free(buf, TRUE);
        }
        g_io_channel_unref(io);
    }
}

static void load_history()
{
    GError *err = NULL;
    GIOChannel *io;
    char *buf = NULL;
    char *b, *c;
    struct HistoryItem *hi = NULL;

    io = g_io_channel_new_file(history_path, "r", &err);
    if (io != NULL) {
        while (g_io_channel_read_line(io, &buf, NULL, NULL, &err) ==
               G_IO_STATUS_NORMAL) {
            hi = g_new0(struct HistoryItem, 1);

            b = buf;
            if ((c = strchr(b, '\t')) == NULL) break;
            *c = '\0';
            hi->name = g_strdup(b);

            b = c + 1;
            if ((c = strchr(b, '\t')) == NULL) break;
            *c = '\0';
            hi->class = g_strdup(b);

            b = c + 1;
            if ((c = strchr(b, '\t')) == NULL) break;
            *c = '\0';
            hi->role = g_strdup(b);

            b = c + 1;
            if ((c = strchr(b, '\t')) == NULL) break;
            *c = '\0';
            hi->x = atoi(b);

            b = c + 1;
            if ((c = strchr(b, '\n')) == NULL) break;
            *c = '\0';
            hi->y = atoi(b);

            hi->placed = FALSE;

            g_free(buf);
            buf = NULL;

            history = g_slist_append(history, hi);
            hi = NULL;
        }
        g_io_channel_unref(io);
    }
        
    g_free(buf);

    if (hi != NULL) {
        g_free(hi->name);
        g_free(hi->class);
        g_free(hi->role);
    }
    g_free(hi);
}

void history_startup()
{
    char *path;

    history = NULL;

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
    for (it = history; it != NULL; it = it->next)
        g_free(it->data);
    g_slist_free(history);

    dispatch_register(0, (EventHandler)event, NULL);

    g_free(history_path);
}
