#include "group.h"
#include "client.h"

GHashTable *group_map = NULL;

static guint map_hash(Window *w) { return *w; }
static gboolean map_key_comp(Window *w1, Window *w2) { return *w1 == *w2; }

void group_startup()
{
    group_map = g_hash_table_new((GHashFunc)map_hash,
                                 (GEqualFunc)map_key_comp);
}

void group_shutdown()
{
    g_hash_table_destroy(group_map);
}

Group *group_add(Window leader, ObClient *client)
{
    Group *self;

    self = g_hash_table_lookup(group_map, &leader);
    if (self == NULL) {
        self = g_new(Group, 1);
        self->leader = leader;
        self->members = NULL;
        g_hash_table_insert(group_map, &self->leader, self);
    }

    self->members = g_slist_append(self->members, client);

    return self;
}

void group_remove(Group *self, ObClient *client)
{
    self->members = g_slist_remove(self->members, client);
    if (self->members == NULL) {
        g_hash_table_remove(group_map, &self->leader);
        g_free(self);
    }
}
