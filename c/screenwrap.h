#ifndef __screenwrap_h
#define __screenwrap_h

#include <Python.h>

/* ScreenWrap is a PyObject */
typedef struct ScreenWrap {
    PyObject_HEAD
} ScreenWrap;

void screenwrap_startup();
void screenwrap_shutdown();

#endif
