#ifndef __group_h
#define __group_h

#include <X11/Xlib.h>
#include <glib.h>

struct _ObClient;

typedef struct Group {
    Window leader;

    /* list of clients */
    GSList *members;
} Group;

extern GHashTable *group_map;

void group_startup();
void group_shutdown();

Group *group_add(Window leader, struct _ObClient *client);

void group_remove(Group *self, struct _ObClient *client);

#endif
