#ifndef __themerc_h
#define __themerc_h

extern char *themerc_engine; /* NULL to use the default engine */
extern char *themerc_theme; /* NULL to use the default theme for the engine */
extern char *themerc_font; /* always non-NULL */
extern char *themerc_titlebar_layout; /* always non-NULL */

void themerc_startup();
void themerc_shutdown();

#endif
