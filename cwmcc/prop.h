#ifndef __prop_h
#define __prop_h

#include <glib.h>
#include <X11/Xlib.h>

/* with the exception of prop_get32, all other prop_get_* functions require
   you to free the returned value with g_free or g_strfreev (for the char**s)
*/

gboolean prop_get32(Window win, Atom prop, Atom type, gulong *ret);

gboolean prop_get_array32(Window win, Atom prop, Atom type, gulong **ret,
                          gulong *nret);

gboolean prop_get_string(Window win, Atom prop, Atom type, char **ret);

/*! Gets a string from a property which is stored in UTF-8 encoding. */
gboolean prop_get_string_utf8(Window win, Atom prop, char **ret);

/*! Gets a string from a property which is stored in the current local
  encoding. The returned string is in UTF-8 encoding. */
gboolean prop_get_string_locale(Window win, Atom prop, char **ret);

/*! Gets a null terminated array of strings from a property which is stored in
  UTF-8 encoding. */
gboolean prop_get_strings_utf8(Window win, Atom prop, char ***ret);

/*! Gets a null terminated array of strings from a property which is stored in
  the current locale encoding. The returned string is in UTF-8 encoding. */
gboolean prop_get_strings_locale(Window win, Atom prop, char ***ret);

/*! Sets a null terminated array of strings in a property encoded as UTF-8. */
void prop_set_strings_utf8(Window win, Atom prop, char **strs);

void prop_erase(Window win, Atom prop);

#endif
