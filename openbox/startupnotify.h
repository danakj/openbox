#ifndef ob__startupnotify_h
#define ob__startupnotify_h

#include <glib.h>

void sn_startup(gboolean reconfig);
void sn_shutdown(gboolean reconfig);

gboolean sn_app_starting();

/*! Notify that an app has started */
void sn_app_started(gchar *wmclass);

/*! Get the desktop requested via the startup-notiication protocol if one
  was requested */
gboolean sn_get_desktop(gchar *id, guint *desktop);

#endif
