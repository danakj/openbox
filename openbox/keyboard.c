#include "focus.h"
#include "openbox.h"
#include "keyboard.h"
#include "clientwrap.h"

#include <Python.h>
#include <glib.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#endif

typedef struct KeyBindingTree {
    guint state;
    guint key;
    GList *keylist;
    PyObject *func;

    /* the next binding in the tree at the same level */
    struct KeyBindingTree *next_sibling; 
    /* the first child of this binding (next binding in a chained sequence).*/
    struct KeyBindingTree *first_child;
} KeyBindingTree;


static KeyBindingTree *firstnode, *curpos;
static guint reset_key, reset_state;
static gboolean grabbed, user_grabbed;
static PyObject *grab_func;

/***************************************************************************
 
   Define the type 'KeyboardData'

 ***************************************************************************/

typedef struct KeyboardData {
    PyObject_HEAD
    PyObject *keychain;
    guint state;
    guint keycode;
    gboolean press;
} KeyboardData;

staticforward PyTypeObject KeyboardDataType;

/***************************************************************************
 
   Type methods/struct
 
 ***************************************************************************/

static PyObject *keybdata_new(PyObject *keychain, guint state,
			      guint keycode, gboolean press)
{
    KeyboardData *data = PyObject_New(KeyboardData, &KeyboardDataType);
    data->keychain = keychain;
    Py_INCREF(keychain);
    data->state = state;
    data->keycode = keycode;
    data->press = press;
    return (PyObject*) data;
}

static void keybdata_dealloc(KeyboardData *self)
{
    Py_DECREF(self->keychain);
    PyObject_Del((PyObject*)self);
}

static PyObject *keybdata_getattr(KeyboardData *self, char *name)
{
    if (!strcmp(name, "keychain")) {
	Py_INCREF(self->keychain);
	return self->keychain;
    } else if (!strcmp(name, "state"))
	return PyInt_FromLong(self->state);
    else if (!strcmp(name, "keycode"))
	return PyInt_FromLong(self->keycode);
    else if (!strcmp(name, "press"))
	return PyInt_FromLong(!!self->press);

    PyErr_Format(PyExc_AttributeError, "no such attribute '%s'", name);
    return NULL;
}

static PyTypeObject KeyboardDataType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "KeyboardData",
    sizeof(KeyboardData),
    0,
    (destructor) keybdata_dealloc,  /*tp_dealloc*/
    0,                              /*tp_print*/
    (getattrfunc) keybdata_getattr, /*tp_getattr*/
    0,                              /*tp_setattr*/
    0,                              /*tp_compare*/
    0,                              /*tp_repr*/
    0,                              /*tp_as_number*/
    0,                              /*tp_as_sequence*/
    0,                              /*tp_as_mapping*/
    0,                              /*tp_hash */
};

/***************************************************************************/



static gboolean grab_keyboard(gboolean grab)
{
    gboolean ret = TRUE;

    g_message("grab_keyboard(%s). grabbed: %d", (grab?"True":"False"),grabbed);

    user_grabbed = grab;
    if (!grabbed) {
	if (grab)
	    ret = XGrabKeyboard(ob_display, ob_root, 0, GrabModeAsync, 
				GrabModeAsync, CurrentTime) == GrabSuccess;
	else
	    XUngrabKeyboard(ob_display, CurrentTime);
    }
    return ret;
}

/***************************************************************************
 
   Define the type 'Keyboard'

 ***************************************************************************/

#define IS_KEYBOARD(v)  ((v)->ob_type == &KeyboardType)
#define CHECK_KEYBOARD(self, funcname) { \
    if (!IS_KEYBOARD(self)) { \
        PyErr_SetString(PyExc_TypeError, \
			"descriptor '" funcname "' requires a 'Keyboard' " \
			"object"); \
	return NULL; \
    } \
}

typedef struct Keyboard {
    PyObject_HEAD
} Keyboard;

staticforward PyTypeObject KeyboardType;


static PyObject *keyb_clearBinds(Keyboard *self, PyObject *args)
{
    CHECK_KEYBOARD(self, "clearBinds");
    if (!PyArg_ParseTuple(args, ":clearBinds"))
	return NULL;
    clearall();
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *keyb_grab(Keyboard *self, PyObject *args)
{
    PyObject *func;

    CHECK_KEYBOARD(self, "grab");
    if (!PyArg_ParseTuple(args, "O:grab", &func))
	return NULL;
    if (!PyCallable_Check(func)) {
	PyErr_SetString(PyExc_ValueError, "expected a callable object");
	return NULL;
    }
    if (!grab_keyboard(TRUE)) {
	PyErr_SetString(PyExc_RuntimeError, "failed to grab keyboard");
	return NULL;
    }
    grab_func = func;
    Py_INCREF(grab_func);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *keyb_ungrab(Keyboard *self, PyObject *args)
{
    CHECK_KEYBOARD(self, "ungrab");
    if (!PyArg_ParseTuple(args, ":ungrab"))
	return NULL;
    grab_keyboard(FALSE);
    Py_XDECREF(grab_func);
    grab_func = NULL;
    Py_INCREF(Py_None);
    return Py_None;
}

#define METH(n, d) {#n, (PyCFunction)keyb_##n, METH_VARARGS, #d}

static PyMethodDef KeyboardMethods[] = {
    METH(bind,
	 "bind(keychain, func)\n\n"
	 "Binds a key-chain to a function. The keychain is a tuple of strings "
	 "which define a chain of key presses. Each member of the tuple has "
	 "the format [Modifier-]...[Key]. Modifiers can be 'mod1', 'mod2', "
	 "'mod3', 'mod4', 'mod5', 'control', and 'shift'. The keys on your "
	 "keyboard that are bound to each of these modifiers can be found by "
	 "running 'xmodmap'. The Key can be any valid key definition. Key "
	 "definitions can be found by running 'xev', pressing the key while "
	 "its window is focused, and watching its output. Here are some "
	 "examples of valid keychains: ('a'), ('F7'), ('control-a', 'd'), "
	 "('control-mod1-x', 'control-mod4-g'), ('F1', 'space'). The func "
	 "must have a definition similar to 'def func(keydata, client)'. A "
	 "keychain cannot be bound to more than one function."),
    METH(clearBinds,
	 "clearBinds()\n\n"
	 "Removes all bindings that were previously made by bind()."),
    METH(grab,
	 "grab(func)\n\n"
	 "Grabs the entire keyboard, causing all possible keyboard events to "
	 "be passed to the given function. CAUTION: Be sure when you grab() "
	 "that you also have an ungrab() that will execute, or you will not "
	 "be able to type until you restart Openbox. The func must have a "
	 "definition similar to 'def func(keydata)'. The keyboard cannot be "
	 "grabbed if it is already grabbed."),
    METH(ungrab,
	 "ungrab()\n\n"
	 "Ungrabs the keyboard. The keyboard cannot be ungrabbed if it is not "
	 "grabbed."),
    { NULL, NULL, 0, NULL }
};

/***************************************************************************
 
   Type methods/struct
 
 ***************************************************************************/

static void keyb_dealloc(PyObject *self)
{
    PyObject_Del(self);
}

static PyTypeObject KeyboardType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Keyboard",
    sizeof(Keyboard),
    0,
    (destructor) keyb_dealloc,      /*tp_dealloc*/
    0,                              /*tp_print*/
    0,                              /*tp_getattr*/
    0,                              /*tp_setattr*/
    0,                              /*tp_compare*/
    0,                              /*tp_repr*/
    0,                              /*tp_as_number*/
    0,                              /*tp_as_sequence*/
    0,                              /*tp_as_mapping*/
    0,                              /*tp_hash */
};

/**************************************************************************/

void keyboard_startup()
{
    PyObject *input, *inputdict, *ptr;
    gboolean b;

    curpos = firstnode = NULL;
    grabbed = user_grabbed = FALSE;

    b = translate("C-G", &reset_state, &reset_key);
    g_assert(b);

    KeyboardType.ob_type = &PyType_Type;
    KeyboardType.tp_methods = KeyboardMethods;
    PyType_Ready(&KeyboardType);
    PyType_Ready(&KeyboardDataType);

    /* get the input module/dict */
    input = PyImport_ImportModule("input"); /* new */
    g_assert(input != NULL);
    inputdict = PyModule_GetDict(input); /* borrowed */
    g_assert(inputdict != NULL);

    /* add a Keyboard instance to the input module */
    ptr = (PyObject*) PyObject_New(Keyboard, &KeyboardType);
    PyDict_SetItemString(inputdict, "Keyboard", ptr);
    Py_DECREF(ptr);

    Py_DECREF(input);
}

void keyboard_shutdown()
{
    if (grabbed || user_grabbed) {
	grabbed = FALSE;
	grab_keyboard(FALSE);
    }
    grab_keys(FALSE);
    destroytree(firstnode);
    firstnode = NULL;
}

