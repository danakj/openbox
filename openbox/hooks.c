#include "hooks.h"
#include <glib.h>

/*
 *
 * Define the 'Hook' class type
 *
 */
#define IS_HOOK(v)  ((v)->ob_type == &HookType)

staticforward PyTypeObject HookType;

typedef struct HookObject {
    PyObject_HEAD
    GSList *funcs;
} HookObject;

static int hook_init(HookObject *self, PyObject *args, PyObject *kwds)
{
    char *keywords[] = { 0 };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, ":__init__", keywords))
	return -1;
    self->funcs = NULL;
    return 0;
}

static void hook_dealloc(HookObject *self)
{
    GSList *it;

    for (it = self->funcs; it != NULL; it = it->next)
	Py_DECREF((PyObject*) it->data);
     
    PyObject_Del((PyObject*) self);
}

static PyObject *hook_fire(HookObject *self, PyObject *args)
{
    GSList *it;

    if (!IS_HOOK(self)) {
	PyErr_SetString(PyExc_TypeError,
			"descriptor 'fire' requires a 'Hook' object");
	return NULL;
    }

    for (it = self->funcs; it != NULL; it = it->next) {
	PyObject *ret = PyObject_CallObject(it->data, args);
	if (ret == NULL)
	    return NULL;
	Py_DECREF(ret);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *hook_append(HookObject *self, PyObject *args)
{
    PyObject *func;
     
    if (!IS_HOOK(self)) {
	PyErr_SetString(PyExc_TypeError,
			"descriptor 'append' requires a 'Hook' object");
	return NULL;
    }
    if (!PyArg_ParseTuple(args, "O:append", &func))
	return NULL;
    if (!PyCallable_Check(func)) {
	PyErr_SetString(PyExc_TypeError,
			"descriptor 'append' requires a callable argument");
	return NULL;
    }
    self->funcs = g_slist_append(self->funcs, func);
    Py_INCREF(func);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *hook_remove(HookObject *self, PyObject *args)
{
    PyObject *func;
    GSList *it;
     
    if (!IS_HOOK(self)) {
	PyErr_SetString(PyExc_TypeError,
			"descriptor 'remove' requires a 'Hook' object");
	return NULL;
    }
    if (!PyArg_ParseTuple(args, "O:remove", &func))
	return NULL;
    if (!PyCallable_Check(func)) {
	PyErr_SetString(PyExc_TypeError,
			"descriptor 'remove' requires a callable argument");
	return NULL;
    }

    it = g_slist_find(self->funcs, func);
    if (it != NULL) {
	self->funcs = g_slist_delete_link(self->funcs, it);
	Py_DECREF(func);

	Py_INCREF(Py_None);
	return Py_None;
    }
    PyErr_SetString(PyExc_TypeError,
		    "given callable object was not found in Hook");
    return NULL;
}

static PyObject *hook_call(HookObject *self, PyObject *args)
{
    GSList *it, *next;
    gboolean stop = FALSE;

    if (!IS_HOOK(self)) {
	PyErr_SetString(PyExc_TypeError,
			"descriptor '__call__' requires a 'Hook' object");
	return NULL;
    }

    for (it = self->funcs; !stop && it != NULL;) {
	next = it->next; /* incase the hook removes itself */

	PyObject *ret = PyObject_CallObject(it->data, args);
	if (ret == NULL)
	    return NULL;
	if (ret != Py_None)
	    stop = TRUE;
	Py_DECREF(ret);

	it = next;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyTypeObject HookType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Hook",
    sizeof(HookObject),
    0,
    (destructor) hook_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
};

static PyMethodDef HookMethods[] = {
    {"append", (PyCFunction)hook_append, METH_VARARGS,
     "hook.add(func) -- Add a function to the hook." },
    {"remove", (PyCFunction)hook_remove, METH_VARARGS,
     "hook.remove(func) -- Remove a function from the hook." },
    { NULL, NULL, 0, NULL }
};


/*
 *
 * Module initialization/finalization
 *
 */

static PyObject *hooks, *hooksdict;

static PyMethodDef HooksMethods[] = {
    { NULL, NULL, 0, NULL }
};

struct HookObject *hooks_create(char *name)
{
    HookObject *hook;
    int ret;

    hook = PyObject_New(HookObject, &HookType);
    hook->funcs = NULL;

    /* add it to the hooks module */
    ret = PyDict_SetItemString(hooksdict, name, (PyObject*) hook);
    g_assert(ret != -1);

    return hook;
}

void hooks_startup()
{
    HookType.ob_type = &PyType_Type;
    HookType.tp_methods = HookMethods;
    HookType.tp_alloc = PyType_GenericAlloc;
    HookType.tp_new = PyType_GenericNew;
    HookType.tp_init = (initproc) hook_init;
    HookType.tp_call = (ternaryfunc) hook_call;
    PyType_Ready(&HookType);

    Py_InitModule("hooks", HooksMethods);

    /* get the hooks module/dict */
    hooks = PyImport_ImportModule("hooks"); /* new */
    g_assert(hooks != NULL);
    hooksdict = PyModule_GetDict(hooks); /* borrowed */
    g_assert(hooksdict != NULL);

    /* add the Hook type to the hooks module */
    PyDict_SetItemString(hooksdict, "Hook", (PyObject*) &HookType);

    hook_startup = hooks_create("startup");
    hook_shutdown = hooks_create("shutdown");
    hook_visibledesktop = hooks_create("visibledesktop");
    hook_numdesktops = hooks_create("numdesktops");
    hook_desktopnames = hooks_create("desktopnames");
    hook_showdesktop = hooks_create("showdesktop");
    hook_screenconfiguration = hooks_create("screenconfiguration");
    hook_screenarea = hooks_create("screenarea");
    hook_managed = hooks_create("managed");
    hook_closed = hooks_create("closed");
    hook_bell = hooks_create("bell");
    hook_urgent = hooks_create("urgent");
    hook_pointerenter = hooks_create("pointerenter");
    hook_pointerleave = hooks_create("pointerleave");
    hook_focused = hooks_create("focused");
    hook_requestactivate = hooks_create("requestactivate");
    hook_title = hooks_create("title");
    hook_desktop = hooks_create("desktop");
    hook_iconic = hooks_create("iconic");
    hook_shaded = hooks_create("shaded");
    hook_maximized = hooks_create("maximized");
    hook_fullscreen = hooks_create("fullscreen");
    hook_visible = hooks_create("visible");
    hook_configuration = hooks_create("configuration");
}

void hooks_shutdown()
{
    Py_DECREF(hook_startup);
    Py_DECREF(hook_shutdown);
    Py_DECREF(hook_visibledesktop);
    Py_DECREF(hook_numdesktops);
    Py_DECREF(hook_desktopnames);
    Py_DECREF(hook_showdesktop);
    Py_DECREF(hook_screenconfiguration);
    Py_DECREF(hook_screenarea);
    Py_DECREF(hook_managed);
    Py_DECREF(hook_closed);
    Py_DECREF(hook_bell);
    Py_DECREF(hook_urgent);
    Py_DECREF(hook_pointerenter);
    Py_DECREF(hook_pointerleave);
    Py_DECREF(hook_focused);
    Py_DECREF(hook_requestactivate);
    Py_DECREF(hook_title);
    Py_DECREF(hook_desktop);
    Py_DECREF(hook_iconic);
    Py_DECREF(hook_shaded);
    Py_DECREF(hook_maximized);
    Py_DECREF(hook_fullscreen);
    Py_DECREF(hook_visible);
    Py_DECREF(hook_configuration);

    Py_DECREF(hooks);
}

void hooks_fire(struct HookObject *hook, PyObject *args)
{
    PyObject *ret = hook_call(hook, args);
    if (ret == NULL)
	PyErr_Print();
    Py_XDECREF(ret);
}

void hooks_fire_client(struct HookObject *hook, struct Client *client)
{
    PyObject *args;

    if (client != NULL) {
	PyObject *c = clientwrap_new(client);
	g_assert(c != NULL);
	args = Py_BuildValue("(O)", c);
	Py_DECREF(c);
    } else {
	args = Py_BuildValue("(O)", Py_None);
    }

    g_assert(args != NULL);
    hooks_fire(hook, args);
    Py_DECREF(args);
}
