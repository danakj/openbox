#ifndef __keyboard_h
#define __keyboard_h

#include <glib.h>

void keyboard_startup();
void keyboard_shutdown();

guint keyboard_translate_modifier(char *str);

void keyboard_event(XKeyEvent *e);

#endif
