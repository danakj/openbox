#ifndef __clientwrap_h
#define __clientwrap_h

#include <Python.h>

struct Client;

/* ClientWrap is a PyObject */
typedef struct ClientWrap {
    PyObject_HEAD
    struct Client *client;
} ClientWrap;

void clientwrap_startup();
void clientwrap_shutdown();

PyObject *clientwrap_new(struct Client *client);

#endif
