#ifndef __group_h
#define __group_h

#include <X11/Xlib.h>
#include <glib.h>

struct Client;

typedef struct Group {
    Window leader;

    /* list of clients */
    GSList *members;
} Group;

extern GHashTable *group_map;

void group_startup();
void group_shutdown();

Group *group_add(Window leader, struct Client *client);

void group_remove(Group *self, struct Client *client);

#endif
