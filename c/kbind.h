#ifndef __kbind_h
#define __kbind_h

#include <glib.h>

void kbind_startup();
void kbind_shutdown();

/*! Adds a new key binding
  A binding will fail to be added if the binding already exists (as part of
  a chain or not), or if any of the strings in the keylist are invalid.    
  @return TRUE if the binding could be added; FALSE if it could not.
*/
gboolean kbind_add(GList *keylist);
void kbind_clearall();

guint kbind_translate_modifier(char *str);

void kbind_fire(guint state, guint key, gboolean press);

gboolean kbind_grab_keyboard(gboolean grab);

#endif
