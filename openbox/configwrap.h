#ifndef __configwrap_h
#define __configwrap_h

void configwrap_startup();
void configwrap_shutdown();

void configwrap_add_int(char *modname, char *varname, char *friendname,
                        char *description, int defvalue);
int configwrap_get_int(char *modname, char *varname);
void configwrap_set_int(char *modname, char *varname, int value);


void configwrap_reset(char *modname, char *varname);

#endif
