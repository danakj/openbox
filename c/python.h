#ifndef __python_h
#define __python_h

void python_startup();
void python_shutdown();

/*! Import a python module */
gboolean python_import(char *module);

#endif
