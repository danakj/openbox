#include "screenwrap.h"
#include "openbox.h"
#include "screen.h"
#include "kbind.h"
#include "mbind.h"

ScreenWrap *screenwrap_instance;

/***************************************************************************
 
   Define the type 'ScreenWrap'

 ***************************************************************************/

#define IS_SWRAP(v)  ((v)->ob_type == &ScreenWrapType)
#define CHECK_SWRAP(self, funcname) { \
    if (!IS_SWRAP(self)) { \
        PyErr_SetString(PyExc_TypeError, \
			"descriptor '" funcname "'  a 'Screen' object"); \
	return NULL; \
    } \
}

staticforward PyTypeObject ScreenWrapType;

/***************************************************************************
 
   Attribute methods
 
 ***************************************************************************/

static PyObject *swrap_number(ScreenWrap *self, PyObject *args)
{
    CHECK_SWRAP(self, "number");
    if (!PyArg_ParseTuple(args, ":number"))
	return NULL;
    return PyInt_FromLong(ob_screen);
}

static PyObject *swrap_rootWindow(ScreenWrap *self, PyObject *args)
{
    CHECK_SWRAP(self, "rootWindow");
    if (!PyArg_ParseTuple(args, ":rootWindow"))
	return NULL;
    return PyInt_FromLong(ob_root);
}

static PyObject *swrap_state(ScreenWrap *self, PyObject *args)
{
    CHECK_SWRAP(self, "state");
    if (!PyArg_ParseTuple(args, ":state"))
	return NULL;
    return PyInt_FromLong(ob_state);
}

static PyObject *swrap_numDesktops(ScreenWrap *self, PyObject *args)
{
    CHECK_SWRAP(self, "numDesktops");
    if (!PyArg_ParseTuple(args, ":numDesktops"))
	return NULL;
    return PyInt_FromLong(screen_num_desktops);
}

static PyObject *swrap_desktop(ScreenWrap *self, PyObject *args)
{
    CHECK_SWRAP(self, "desktop");
    if (!PyArg_ParseTuple(args, ":desktop"))
	return NULL;
    return PyInt_FromLong(screen_desktop);
}

static PyObject *swrap_physicalSize(ScreenWrap *self, PyObject *args)
{
    PyObject *tuple;

    CHECK_SWRAP(self, "physicalSize");
    if (!PyArg_ParseTuple(args, ":physicalSize"))
	return NULL;
    tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(screen_physical_size.width));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(screen_physical_size.height));
    return tuple;
}

static PyObject *swrap_showingDesktop(ScreenWrap *self, PyObject *args)
{
    CHECK_SWRAP(self, "showingDesktop");
    if (!PyArg_ParseTuple(args, ":showingDesktop"))
	return NULL;
    return PyInt_FromLong(screen_showing_desktop ? 1 : 0);
}

static PyObject *swrap_desktopLayout(ScreenWrap *self, PyObject *args)
{
    PyObject *tuple;

    CHECK_SWRAP(self, "desktopLayout");
    if (!PyArg_ParseTuple(args, ":desktopLayout"))
	return NULL;
    tuple = PyTuple_New(4);
    PyTuple_SET_ITEM(tuple, 0,
		     PyInt_FromLong(screen_desktop_layout.orientation));
    PyTuple_SET_ITEM(tuple, 1,
		     PyInt_FromLong(screen_desktop_layout.start_corner));
    PyTuple_SET_ITEM(tuple, 2,
		     PyInt_FromLong(screen_desktop_layout.rows));
    PyTuple_SET_ITEM(tuple, 3,
		     PyInt_FromLong(screen_desktop_layout.columns));
    return tuple;
}

static PyObject *swrap_desktopNames(ScreenWrap *self, PyObject *args)
{
    PyObject *list;
    guint s, i;

    CHECK_SWRAP(self, "desktopNames");
    if (!PyArg_ParseTuple(args, ":desktopNames"))
	return NULL;
    s = screen_desktop_names->len;
    list = PyList_New(s);
    for (i = 0; i < s; ++i)
	PyList_SET_ITEM(list, i, PyString_FromString
			(g_ptr_array_index(screen_desktop_names, i)));
    return list;
}

static PyObject *swrap_area(ScreenWrap *self, PyObject *args)
{
    PyObject * tuple;
    Rect *r;
    guint i;

    CHECK_SWRAP(self, "area");
    if (!PyArg_ParseTuple(args, "i:area", &i))
	return NULL;
    r = screen_area(i);
    if (r == NULL) {
	PyErr_SetString(PyExc_IndexError,
			"the requested desktop was not valid");
	return NULL;
    }
    tuple = PyTuple_New(4);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(r->x));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(r->y));
    PyTuple_SET_ITEM(tuple, 2, PyInt_FromLong(r->width));
    PyTuple_SET_ITEM(tuple, 3, PyInt_FromLong(r->height));
    return tuple;
}

static PyObject *swrap_strut(ScreenWrap *self, PyObject *args)
{
    PyObject *tuple;
    Strut *s;
    guint i;

    CHECK_SWRAP(self, "strut");
    if (!PyArg_ParseTuple(args, "i:strut", &i))
	return NULL;
    s = screen_strut(i);
    if (s == NULL) {
	PyErr_SetString(PyExc_IndexError,
			"the requested desktop was not valid");
	return NULL;
    }
    tuple = PyTuple_New(4);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(s->left));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(s->top));
    PyTuple_SET_ITEM(tuple, 2, PyInt_FromLong(s->right));
    PyTuple_SET_ITEM(tuple, 3, PyInt_FromLong(s->bottom));
    return tuple;
}

static PyObject *swrap_grabKey(ScreenWrap *self, PyObject *args)
{
    PyObject *item, *tuple;
    GList *keylist = NULL, *it;
    int i, s;
    gboolean grab = FALSE;

    CHECK_SWRAP(self, "grabKey");
    if (!PyArg_ParseTuple(args, "O:grabKey", &tuple))
	return NULL;

    if (PyTuple_Check(tuple)) {
	s = PyTuple_GET_SIZE(tuple);
	if (s > 0) {
	    for (i = 0; i < s; ++i) {
		item = PyTuple_GET_ITEM(tuple, i);
		if (!PyString_Check(item))
		    break;
		keylist = g_list_append(keylist,
					g_strdup(PyString_AsString(item)));
	    }
	    if (i == s)
		grab = kbind_add(keylist);

	    for (it = keylist; it != NULL; it = it->next)
		g_free(it->data);
	    g_list_free(it);

	    if (grab) {
		Py_INCREF(Py_None);
		return Py_None;
	    } else {
		PyErr_SetString(PyExc_ValueError,
				"the key could not be grabbed");
		return NULL;
	    }
	}
    }

    PyErr_SetString(PyExc_TypeError, "expected a tuple of strings");
    return NULL;
}

static PyObject *swrap_clearKeyGrabs(ScreenWrap *self, PyObject *args)
{
    CHECK_SWRAP(self, "clearKeyGrabs");
    if (!PyArg_ParseTuple(args, ":clearKeyGrabs"))
	return NULL;
    kbind_clearall();
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *swrap_grabKeyboard(ScreenWrap *self, PyObject *args)
{
    int grab;

    CHECK_SWRAP(self, "grabKeyboard");
    if (!PyArg_ParseTuple(args, "i:grabKeyboard", &grab))
	return NULL;
    if (!kbind_grab_keyboard(grab)) {
	PyErr_SetString(PyExc_RuntimeError, "failed to grab keyboard");
	return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *swrap_grabButton(ScreenWrap *self, PyObject *args)
{
    char *name;
    char *context_str;
    GQuark context;

    CHECK_SWRAP(self, "grabButton");
    if (!PyArg_ParseTuple(args, "ss:grabKey", &name, &context_str))
	return NULL;

    context = g_quark_try_string(context_str);

    if (!context) {
	PyErr_SetString(PyExc_ValueError, "invalid context");
	return NULL;
    }

    if (mbind_add(name, context)) {
	Py_INCREF(Py_None);
	return Py_None;
    } else {
	PyErr_SetString(PyExc_ValueError,
			"the button could not be grabbed");
	return NULL;
    }
}

static PyObject *swrap_clearButtonGrabs(ScreenWrap *self, PyObject *args)
{
    CHECK_SWRAP(self, "clearButtonGrabs");
    if (!PyArg_ParseTuple(args, ":clearButtonGrabs"))
	return NULL;
    mbind_clearall();
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *swrap_grabPointer(ScreenWrap *self, PyObject *args)
{
    int grab;

    CHECK_SWRAP(self, "grabPointer");
    if (!PyArg_ParseTuple(args, "i:grabPointer", &grab))
	return NULL;
    if (!mbind_grab_pointer(grab)) {
	PyErr_SetString(PyExc_RuntimeError, "failed to grab pointer");
	return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef ScreenWrapAttributeMethods[] = {
    {"number", (PyCFunction)swrap_number, METH_VARARGS,
     "Screen.number() -- Returns the number of the screen on the X server on "
     "which this Openbox instance is running."},
    {"rootWindow", (PyCFunction)swrap_rootWindow, METH_VARARGS,
     "Screen.rootWindow() -- Returns the window id of the root window."},
    {"state", (PyCFunction)swrap_state, METH_VARARGS,
     "Screen.state() -- Returns the running state of Openbox. One of the "
     "ob.State_ constants."},
    {"numDesktops", (PyCFunction)swrap_numDesktops, METH_VARARGS,
     "Screen.numDesktops() -- Returns the number of desktops available."},
    {"desktop", (PyCFunction)swrap_desktop, METH_VARARGS,
     "Screen.desktop() -- Returns the currently visible desktop."},
    {"physicalSize", (PyCFunction)swrap_physicalSize, METH_VARARGS,
     "Screen.physicalSize() -- Returns the physical size (in pixels) of the "
     "display in a tuple. The tuple is formatted as (width, height)."},
    {"showingDesktop", (PyCFunction)swrap_showingDesktop, METH_VARARGS,
     "Screen.showingDesktop() -- Returns if in showing-the-desktop mode or "
     "not."},
    {"desktopNames", (PyCFunction)swrap_desktopNames, METH_VARARGS,
     "Screen.desktopNames() -- Returns a list of the names of all the "
     "desktops, and possibly for desktops beyond those. The names are encoded "
     "as UTF-8."},
    {"desktopLayout", (PyCFunction)swrap_desktopLayout, METH_VARARGS,
     "Screen.desktopLayout() -- Returns the layout of the desktops, as "
     "specified by a compliant pager, in a tuple. The format of the tuple is "
     "(orientation, corner, rows, columns). Where, orientation is one of the "
     "ob.Orientation_ constants, corner is one of the ob.Corner_ constants, "
     "and rows and columns specify the size of the layout. The rows and "
     "columns will always include all the desktops."},
    {"area", (PyCFunction)swrap_area, METH_VARARGS,
     "Screen.area(d) -- Returns the usuable area on the Screen for a desktop, "
     "in the form of a tuple. The tuples format is (x, y, width, height). The "
     "desktop must be in the range of desktops on the screen, or 0xffffffff "
     "to get a combined area for 'all desktops'."},
    {"strut", (PyCFunction)swrap_area, METH_VARARGS,
     "Screen.strut(d) -- Returns the combined strut of all clients on a "
     "desktop, in the form of a tuple. The tuples format is "
     "(left, top, right, bottom). The desktop must be in the range of "
     "desktops on the screen, or 0xffffffff to get a combined strut for "
     "'all desktops'."},
    {"grabKey", (PyCFunction)swrap_grabKey, METH_VARARGS,
     "Screen.grabKey(('Mod1-C-a', 'd')) -- Grabs a key chain so that key "
     "events for it will occur. The argument must be a tuple of one or "
     "more elements. Each key element is made up of "
     "Modifier-Modifier-...-Key, where Modifier is one of Mod1, Mod2, "
     "Mod3, Mod4, Mod5, S (for Shift), or C (for Control)."},
    {"clearKeyGrabs", (PyCFunction)swrap_clearKeyGrabs, METH_VARARGS,
     "Screen.clearKeyGrabs() -- Removes all key grabs that have been done "
     "with grabKey()."},
    {"grabKeyboard", (PyCFunction)swrap_grabKeyboard, METH_VARARGS,
     "Screen.grabKeyboard(grab) -- Grabs or ungrabs the entire keyboard. When "
     "the keyboard is grabbed, all key presses will be sent to the "
     "hooks.keyboard hook. (grabbed keys will go to the hooks.events hook "
     "too. "},
    {"grabButton", (PyCFunction)swrap_grabButton, METH_VARARGS,
     "Screen.grabButton('C-1', \"frame\") -- Grabs a pointer button "
     "for the given context. The context must be one of the ob.Context_* "
     "constants. The button definition is made up of "
     "Modifier-Modifier-...-Button, where Modifier is one of Mod1, Mod2, "
     "Mod3, Mod4, Mod5, S (for Shift), or C (for Control)."},
    {"clearButtonGrabs", (PyCFunction)swrap_clearButtonGrabs, METH_VARARGS,
     "Screen.clearButtonGrabs() -- Removes all button grabs that have been "
     "done with grabButton()."},
    {"grabPointer", (PyCFunction)swrap_grabPointer, METH_VARARGS,
     "grabPointer(grab) -- Grabs or ungrabs the pointer device. When the "
     "pointer is grabbed, all pointer events will be sent to the "
     "hooks.pointer  hook. (grabbed buttons will NOT go to the hooks.events "
     "hook while the pointer is grabbed)."},
    {NULL, NULL, 0, NULL}
};

/***************************************************************************
 
   Type methods/struct
 
 ***************************************************************************/

static PyObject *swrap_getattr(ScreenWrap *self, char *name)
{
    CHECK_SWRAP(self, "getattr");
    return Py_FindMethod(ScreenWrapAttributeMethods, (PyObject*)self, name);
}

static void swrap_dealloc(ScreenWrap *self)
{
    PyObject_Del((PyObject*) self);
}

static PyTypeObject ScreenWrapType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Screen",
    sizeof(ScreenWrap),
    0,
    (destructor) swrap_dealloc,     /*tp_dealloc*/
    0,                              /*tp_print*/
    (getattrfunc) swrap_getattr,    /*tp_getattr*/
    0,                              /*tp_setattr*/
    0,                              /*tp_compare*/
    0,                              /*tp_repr*/
    0,                              /*tp_as_number*/
    0,                              /*tp_as_sequence*/
    0,                              /*tp_as_mapping*/
    0,                              /*tp_hash */
};

/***************************************************************************
 
   External methods
 
 ***************************************************************************/

void screenwrap_startup()
{
    PyObject *ob, *obdict;
    ScreenWrap *swrap;

    ScreenWrapType.ob_type = &PyType_Type;
    ScreenWrapType.tp_doc = "Wraps information and functionality global to an "
	"instance of Openbox.";
    PyType_Ready(&ScreenWrapType);
    swrap = PyObject_New(ScreenWrap, &ScreenWrapType);

    /* get the ob module/dict */
    ob = PyImport_ImportModule("ob"); /* new */
    g_assert(ob != NULL);
    obdict = PyModule_GetDict(ob); /* borrowed */
    g_assert(obdict != NULL);

    PyDict_SetItemString(obdict, "Screen", (PyObject*)swrap);
    Py_DECREF(swrap);
    Py_DECREF(ob);
}

void screenwrap_shutdown()
{
}


