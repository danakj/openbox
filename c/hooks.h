#ifndef __hooks_h
#define __hooks_h

#include "eventdata.h"

void hooks_startup();
void hooks_shutdown();

void hooks_fire(EventData *data);

void hooks_fire_keyboard(EventData *data);

void hooks_fire_pointer(EventData *data);

#define LOGICALHOOK(type, context, client) \
{ EventData *data = eventdata_new_logical(Logical_##type, \
					  context, client); \
  g_assert(data != NULL); \
  hooks_fire(data); \
  eventdata_free(data); \
}

#endif
