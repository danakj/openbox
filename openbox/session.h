#ifndef __ob__session_h
#define __ob__session_h

#include <glib.h>

struct _ObClient;

typedef struct _ObSessionState ObSessionState;

struct _ObSessionState {
    gchar *id, *name, *class, *role;
    guint stacking;
    guint desktop;
    gint x, y, w, h;
    gboolean shaded, iconic, skip_pager, skip_taskbar, fullscreen;
    gboolean above, below, max_horz, max_vert;

    gboolean matched;
};

extern GList *session_saved_state;

void session_load(char *path);
void session_startup(int argc, char **argv);
void session_shutdown();

GList* session_state_find(struct _ObClient *c);
gboolean session_state_cmp(ObSessionState *s, struct _ObClient *c);
void session_state_free(ObSessionState *state);

#endif
