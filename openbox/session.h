#ifndef __ob__session_h
#define __ob__session_h

#include <glib.h>

struct _ObClient;

typedef struct _ObSessionState ObSessionState;

struct _ObSessionState {
    gchar *id, *name, *class, *role;
    guint desktop;
    gint x, y, w, h;
    gboolean shaded, iconic, skip_pager, skip_taskbar, fullscreen;
    gboolean above, below, max_horz, max_vert;
};


void session_load(char *path);
void session_startup(int argc, char **argv);
void session_shutdown();

ObSessionState* session_state_find(struct _ObClient *c);
void session_state_free(ObSessionState *state);

#endif
