#ifndef ob__translate_h
#define ob__translate_h

#include <glib.h>

gboolean translate_button(gchar *str, guint *state, guint *keycode);
gboolean translate_key(gchar *str, guint *state, guint *keycode);

#endif
