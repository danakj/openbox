#include <Python.h>
#include <glib.h>

/* This simply wraps the config.py module so that it can be accessed from the
   C code.
*/

static PyObject *add, *get, *set, *reset;

void configwrap_startup()
{
    PyObject *c, *cdict;

    /* get the ob module/dict */
    c = PyImport_ImportModule("config"); /* new */
    g_assert(c != NULL);
    cdict = PyModule_GetDict(c); /* borrowed */
    g_assert(cdict != NULL);

    /* get the functions */
    add = PyDict_GetItemString(cdict, "add");
    g_assert(add != NULL);
    get = PyDict_GetItemString(cdict, "get");
    g_assert(get != NULL);
    set = PyDict_GetItemString(cdict, "set");
    g_assert(set != NULL);
    reset = PyDict_GetItemString(cdict, "reset");
    g_assert(reset != NULL);

    Py_DECREF(c);
}

void configwrap_shutdown()
{
    Py_DECREF(get);
    Py_DECREF(set);
    Py_DECREF(reset);
    Py_DECREF(add);
}

void configwrap_add_int(char *modname, char *varname, char *friendname,
                             char *description, int defvalue)
{
    PyObject *r;

    r= PyObject_CallFunction(add, "sssssi", modname, varname,
                             friendname, description, "integer", defvalue);
    g_assert(r != NULL);
    Py_DECREF(r);
}

int configwrap_get_int(char *modname, char *varname)
{
    PyObject *r;
    int i;

    r = PyObject_CallFunction(get, "ss", modname, varname);
    g_assert(r != NULL);
    i = PyInt_AsLong(r);
    Py_DECREF(r);
    return i;
}

void configwrap_set_int(char *modname, char *varname, int value)
{
    PyObject *r;

    r = PyObject_CallFunction(set, "ssi", modname, varname, value);
    g_assert(r != NULL);
    Py_DECREF(r);
}

void configwrap_reset(char *modname, char *varname)
{
    PyObject *r;

    r = PyObject_CallFunction(reset, "ss", modname, varname);
    g_assert(r != NULL);
    Py_DECREF(r);
}
