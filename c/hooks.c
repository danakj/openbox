#include "hooks.h"
#include <Python.h>
#include <glib.h>

/* the 'hooks' module and its dictionary */
static PyObject *hooks = NULL, *hooksdict = NULL;

/*
 *
 * Define the type 'Hook'
 *
 */
#define IS_HOOK(v)  ((v)->ob_type == &HookType)

staticforward PyTypeObject HookType;

typedef struct {
    PyObject_HEAD
    GSList *funcs;
} HookObject;

static PyObject *create_Hook(PyObject *self, PyObject *args)
{
    HookObject *hook;
    char *name;
    int ret;

    (void) self;

    if (!PyArg_ParseTuple(args, "s:Hook", &name))
	return NULL;

    hook = PyObject_New(HookObject, &HookType);
    hook->funcs = NULL;

    /* add it to the hooks module */
    ret = PyDict_SetItemString(hooksdict, name, (PyObject*) hook);
    Py_DECREF(hook);

    if (ret == -1) {
	char *s = g_strdup_printf(
	    "Failed to add the hook '%s' to the 'hooks' module", name);
	PyErr_SetString(PyExc_RuntimeError, s);
	g_free(s);
	return NULL;
    }
     
    Py_INCREF(Py_None);
    return Py_None;
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

static PyObject *hook_add(HookObject *self, PyObject *args)
{
    PyObject *func;
     
    if (!IS_HOOK(self)) {
	PyErr_SetString(PyExc_TypeError,
			"descriptor 'add' requires a 'Hook' object");
	return NULL;
    }
    if (!PyArg_ParseTuple(args, "O:add", &func))
	return NULL;
    if (!PyCallable_Check(func)) {
	PyErr_SetString(PyExc_TypeError,
			"descriptor 'add' requires a callable argument");
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

static PyObject *hook_count(HookObject *self, PyObject *args)
{
    if (!IS_HOOK(self)) {
	PyErr_SetString(PyExc_TypeError,
			"descriptor 'fire' requires a 'Hook' object");
	return NULL;
    }
    if (!PyArg_ParseTuple(args, ":count"))
	return NULL;

    return PyInt_FromLong(g_slist_length(self->funcs));
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
    {"fire", (PyCFunction)hook_fire, METH_VARARGS,
     "hook.fire() -- Fire the added hook functions for the Hook."},
    {"add", (PyCFunction)hook_add, METH_VARARGS,
     "hook.add(func) -- Add a function to the hook." },
    {"remove", (PyCFunction)hook_remove, METH_VARARGS,
     "hook.remove(func) -- Remove a function from the hook." },
    {"count", (PyCFunction)hook_count, METH_VARARGS,
     "hook.count() -- Return the number of functions in the hook." },
     
    { NULL, NULL, 0, NULL }
};


/*
 *
 * Module initialization/finalization
 *
 */

/* the "events" hook */
static HookObject *events_hook = NULL, *keyboard_hook = NULL,
    *pointer_hook = NULL;

static PyMethodDef HooksMethods[] = {
    {"create", create_Hook, METH_VARARGS,
     "hooks.create('name') -- Add a hook called 'name' to the hooks module."},
     
    { NULL, NULL, 0, NULL }
};

void hooks_startup()
{
    int ret;

    HookType.ob_type = &PyType_Type;
    HookType.tp_methods = HookMethods;
    PyType_Ready(&HookType);

    Py_InitModule("hooks", HooksMethods);

    /* get the hooks module/dict */
    hooks = PyImport_ImportModule("hooks"); /* new */
    g_assert(hooks != NULL);
    hooksdict = PyModule_GetDict(hooks); /* borrowed */
    g_assert(hooksdict != NULL);

    /* create the "events" hook */
    events_hook = PyObject_New(HookObject, &HookType);
    events_hook->funcs = NULL;

    /* add it to the hooks module */
    ret = PyDict_SetItemString(hooksdict, "events", (PyObject*) events_hook);
    g_assert(ret == 0);

    /* create the "keyboard" hook */
    keyboard_hook = PyObject_New(HookObject, &HookType);
    keyboard_hook->funcs = NULL;

    /* add it to the hooks module */
    ret = PyDict_SetItemString(hooksdict, "keyboard",
			       (PyObject*) keyboard_hook);
    g_assert(ret == 0);

    /* create the "pointer" hook */
    pointer_hook = PyObject_New(HookObject, &HookType);
    pointer_hook->funcs = NULL;

    /* add it to the hooks module */
    ret = PyDict_SetItemString(hooksdict, "pointer", (PyObject*) pointer_hook);
    g_assert(ret == 0);
}

void hooks_shutdown()
{
    Py_DECREF(pointer_hook);
    Py_DECREF(keyboard_hook);
    Py_DECREF(events_hook);
    Py_DECREF(hooks);
}

void hooks_fire(EventData *data)
{
    PyObject *ret, *args;

    g_assert(events_hook != NULL);

    args = Py_BuildValue("(O)", data);
    ret = hook_fire(events_hook, args);
    Py_DECREF(args);
    if (ret == NULL)
	PyErr_Print();
}

void hooks_fire_keyboard(EventData *data)
{
    PyObject *ret, *args;

    g_assert(events_hook != NULL);

    args = Py_BuildValue("(O)", data);
    ret = hook_fire(keyboard_hook, args);
    Py_DECREF(args);
    if (ret == NULL)
	PyErr_Print();
}

void hooks_fire_pointer(EventData *data)
{
    PyObject *ret, *args;

    g_assert(events_hook != NULL);

    args = Py_BuildValue("(O)", data);
    ret = hook_fire(pointer_hook, args);
    Py_DECREF(args);
    if (ret == NULL)
	PyErr_Print();
}
