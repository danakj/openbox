#ifndef __openboxwrap_h
#define __openboxwrap_h

#include <Python.h>

/* OpenboxWrap is a PyObject */
typedef struct OpenboxWrap {
    PyObject_HEAD
    PyObject *client_list;
} OpenboxWrap;

OpenboxWrap *openboxwrap_obj;

void openboxwrap_startup();
void openboxwrap_shutdown();

#endif
