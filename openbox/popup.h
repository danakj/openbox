#ifndef __popup_h
#define __popup_h

#include "client.h"

#include <glib.h>

typedef struct _ObPopup Popup;

Popup *popup_new(gboolean hasicon);
void popup_free(Popup *self);

/*! Position the popup. The gravity rules are not the same X uses for windows,
  instead of the position being the top-left of the window, the gravity
  specifies which corner of the popup will be placed at the given coords.
  Static and Forget gravity are equivilent to NorthWest.
*/
void popup_position(Popup *self, gint gravity, gint x, gint y);
/*! Set the sizes for the popup. When set to 0, the size will be based on
  the text size. */
void popup_size(Popup *self, gint w, gint h);
void popup_size_to_string(Popup *self, gchar *text);

void popup_show(Popup *self, gchar *text, ObClientIcon *icon);
void popup_hide(Popup *self);

#endif
