#ifndef __startup_h
#define __startup_h

extern guint32 *startup_stack_order;
extern guint    startup_stack_size;
extern guint32  startup_active;
extern guint32  startup_desktop;

void startup_save();

#endif
