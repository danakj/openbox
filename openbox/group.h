#ifndef __group_h
#define __group_h

#include <X11/Xlib.h>
#include <glib.h>

typedef struct _ObGroup ObGroup;

struct _ObClient;

struct _ObGroup
{
    Window leader;

    /* list of clients */
    GSList *members;
};

extern GHashTable *group_map;

void group_startup();
void group_shutdown();

ObGroup *group_add(Window leader, struct _ObClient *client);

void group_remove(ObGroup *self, struct _ObClient *client);

#endif
