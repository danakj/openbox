#include "openboxwrap.h"
#include "openbox.h"
#include "screen.h"
#include "prop.h"

/***************************************************************************
 
   Define the type 'OpenboxWrap'

 ***************************************************************************/

#define IS_OWRAP(v)  ((v)->ob_type == &OpenboxWrapType)
#define CHECK_OWRAP(self, funcname) { \
    if (!IS_OWRAP(self)) { \
        PyErr_SetString(PyExc_TypeError, \
			"descriptor '" funcname "' requires a 'Openbox' " \
			"object"); \
	return NULL; \
    } \
}


staticforward PyTypeObject OpenboxWrapType;

/***************************************************************************
 
   Attribute methods
 
 ***************************************************************************/

static PyObject *owrap_shutdown(OpenboxWrap *self, PyObject *args)
{
    CHECK_OWRAP(self, "shutdown");
    if (!PyArg_ParseTuple(args, ":shutdown"))
	return NULL;
    ob_shutdown = TRUE;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *owrap_restart(OpenboxWrap *self, PyObject *args)
{
    char *path = NULL;

    CHECK_OWRAP(self, "restart");
    if (!PyArg_ParseTuple(args, "|s:restart", &path))
	return NULL;
    ob_shutdown = ob_restart = TRUE;
    ob_restart_path = path;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *owrap_state(OpenboxWrap *self, PyObject *args)
{
    CHECK_OWRAP(self, "state");
    if (!PyArg_ParseTuple(args, ":state"))
	return NULL;
    return PyInt_FromLong(ob_state);
}

static PyObject *owrap_desktop(OpenboxWrap *self, PyObject *args)
{
    CHECK_OWRAP(self, "desktop");
    if (!PyArg_ParseTuple(args, ":desktop"))
	return NULL;
    return PyInt_FromLong(screen_desktop);
}

static PyObject *owrap_setDesktop(OpenboxWrap *self, PyObject *args)
{
    int desktop;

    CHECK_OWRAP(self, "setDesktop");
    if (!PyArg_ParseTuple(args, "i:setDesktop", &desktop))
	return NULL;
    if (desktop < 0 || (unsigned)desktop >= screen_num_desktops) {
	PyErr_SetString(PyExc_ValueError, "invalid desktop");
	return NULL;
    }
    screen_set_desktop(desktop);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *owrap_setNextDesktop(OpenboxWrap *self, PyObject *args)
{
    gboolean wrap = TRUE;
    guint d;

    CHECK_OWRAP(self, "setNextDesktop");
    if (!PyArg_ParseTuple(args, "|i:setNextDesktop", &wrap))
	return NULL;
    d = screen_desktop + 1;
    if (d >= screen_num_desktops && wrap)
        d = 0;
    if (d < screen_num_desktops)
        screen_set_desktop(d);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *owrap_setPreviousDesktop(OpenboxWrap *self, PyObject *args)
{
    gboolean wrap = TRUE;
    guint d;

    CHECK_OWRAP(self, "setPreviousDesktop");
    if (!PyArg_ParseTuple(args, "|i:setPreviousDesktop", &wrap))
	return NULL;
    d = screen_desktop - 1;
    if (d >= screen_num_desktops && wrap)
        d = screen_num_desktops - 1;
    if (d < screen_num_desktops)
        screen_set_desktop(d);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *owrap_numDesktops(OpenboxWrap *self, PyObject *args)
{
    CHECK_OWRAP(self, "numDesktops");
    if (!PyArg_ParseTuple(args, ":numDesktops"))
	return NULL;
    return PyInt_FromLong(screen_num_desktops);
}

static PyObject *owrap_setNumDesktops(OpenboxWrap *self, PyObject *args)
{
    int desktops;

    CHECK_OWRAP(self, "setNumDesktops");
    if (!PyArg_ParseTuple(args, "i:setNumDesktops", &desktops))
	return NULL;
    if (desktops <= 0) {
	PyErr_SetString(PyExc_ValueError, "invalid number of desktops");
	return NULL;
    }
    screen_set_num_desktops(desktops);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *owrap_desktopNames(OpenboxWrap *self, PyObject *args)
{
    PyObject *tuple;
    int i, s;

    CHECK_OWRAP(self, "desktopNames");
    if (!PyArg_ParseTuple(args, ":desktopNames"))
	return NULL;
    s = screen_desktop_names->len;
    tuple = PyTuple_New(s);
    for (i = 0; i < s; ++i)
	PyTuple_SET_ITEM(tuple, i, g_ptr_array_index(screen_desktop_names, i));
    return tuple;
}

static PyObject *owrap_setDesktopNames(OpenboxWrap *self, PyObject *args)
{
    PyObject *seq;
    int i, s;
    GPtrArray *data;

    CHECK_OWRAP(self, "setDesktopNames");
    if (!PyArg_ParseTuple(args, "O:setDesktopNames", &seq))
	return NULL;
    if (!PySequence_Check(seq))
	PyErr_SetString(PyExc_TypeError, "expected a sequence");
    return NULL;

    s = PySequence_Size(seq);
    for (i = 0; i < s; ++i) {
	PyObject *item;
	gboolean check;
	item = PySequence_GetItem(seq, i); /* new */
	check = PyString_Check(item);
	Py_DECREF(item);
	if (!check) {
	    PyErr_SetString(PyExc_TypeError, "expected a sequence of strings");
	    return NULL;
	}
    }

    data = g_ptr_array_sized_new(s);
    for (i = 0; i < s; ++i) {
	PyObject *item;
	item = PySequence_GetItem(seq, i); /* new */
	g_ptr_array_index(data, i) = PyString_AsString(item); /* borrowed */
	Py_DECREF(item);
    }

    PROP_SETSA(ob_root, net_desktop_names, utf8, data);
    g_ptr_array_free(data, TRUE);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *owrap_showingDesktop(OpenboxWrap *self, PyObject *args)
{
    CHECK_OWRAP(self, "showingDesktop");
    if (!PyArg_ParseTuple(args, ":showingDesktop"))
	return NULL;
    return PyInt_FromLong(!!screen_showing_desktop);
}

static PyObject *owrap_setShowingDesktop(OpenboxWrap *self, PyObject *args)
{
    int show;

    CHECK_OWRAP(self, "setShowingDesktop");
    if (!PyArg_ParseTuple(args, "i:setShowingDesktop", &show))
	return NULL;
    screen_show_desktop(show);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *owrap_screenArea(OpenboxWrap *self, PyObject *args)
{
    int desktop;
    Rect *area;
    PyObject *tuple;

    CHECK_OWRAP(self, "screenArea");
    if (!PyArg_ParseTuple(args, "i:screenArea", &desktop))
	return NULL;

    area = screen_area(desktop);
    if (area == NULL) {
	PyErr_SetString(PyExc_ValueError, "invalid desktop");
	return NULL;
    }

    tuple = PyTuple_New(4);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(area->x));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(area->y));
    PyTuple_SET_ITEM(tuple, 2, PyInt_FromLong(area->width));
    PyTuple_SET_ITEM(tuple, 3, PyInt_FromLong(area->height));
    return tuple;
}

static PyObject *owrap_screenStrut(OpenboxWrap *self, PyObject *args)
{
    int desktop;
    Strut *strut;
    PyObject *tuple;

    CHECK_OWRAP(self, "screenStrut");
    if (!PyArg_ParseTuple(args, "i:screenStrut", &desktop))
	return NULL;

    strut = screen_strut(desktop);
    if (strut == NULL) {
	PyErr_SetString(PyExc_ValueError, "invalid desktop");
	return NULL;
    }

    tuple = PyTuple_New(4);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(strut->left));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(strut->top));
    PyTuple_SET_ITEM(tuple, 2, PyInt_FromLong(strut->right));
    PyTuple_SET_ITEM(tuple, 3, PyInt_FromLong(strut->bottom));
    return tuple;
}

static PyObject *owrap_physicalSize(OpenboxWrap *self, PyObject *args)
{
    PyObject *tuple;

    CHECK_OWRAP(self, "physicalSize");
    if (!PyArg_ParseTuple(args, ":physicalSize"))
	return NULL;

    tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(screen_physical_size.width));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(screen_physical_size.height));
    return tuple;
}

static PyObject *owrap_screenNumber(OpenboxWrap *self, PyObject *args)
{
    CHECK_OWRAP(self, "screenNumber");
    if (!PyArg_ParseTuple(args, ":screenNumber"))
	return NULL;
    return PyInt_FromLong(ob_screen);
}

static PyObject *owrap_rootWindow(OpenboxWrap *self, PyObject *args)
{
    CHECK_OWRAP(self, "rootWindow");
    if (!PyArg_ParseTuple(args, ":rootWindow"))
	return NULL;
    return PyInt_FromLong(ob_root);
}

static PyObject *owrap_clientList(OpenboxWrap *self, PyObject *args)
{
    CHECK_OWRAP(self, "clientList");
    if (!PyArg_ParseTuple(args, ":clientList"))
	return NULL;
    Py_INCREF(self->client_list);
    return self->client_list;
}

#define METH(n, d) {#n, (PyCFunction)owrap_##n, METH_VARARGS, #d}

static PyMethodDef OpenboxWrapMethods[] = {
    METH(shutdown,
	 "Causes Openbox to shutdown and exit."),
    METH(restart,
	 "Causes Openbox to shutdown and restart. If path is specified, "
	 "Openbox will shutdown and attempt to run the specified executable "
	 "instead of restarting itself. If that fails, however, it will "
	 "restart itself."),
    METH(state,
	 "Returns Openbox's current state, this will be one of the State "
	 "constants."),
    METH(desktop,
	 "Returns the number of the currently visible desktop. This will be "
	 "in the range of [0, numDesktops())."),
    METH(setDesktop,
	 "Sets the specified desktop as the visible desktop."),
    METH(setNextDesktop,
         "Sets the visible desktop to the next desktop, optionally wrapping "
         "around when reaching the last."),
    METH(setPreviousDesktop,
         "Sets the visible desktop to the previous desktop, optionally "
         "wrapping around when reaching the first."),
    METH(numDesktops,
	 "Returns the number of desktops available."),
    METH(desktopNames,
	 "Returns a tuple of names, containing a name for each desktop. The "
	 "tuple may have a length greater than numDesktops() if more names "
	 "have been specified."),
    METH(setDesktopNames,
	 "Sets the names for the desktops."),
    METH(showingDesktop,
	 "Returns True or False, depicting if Openbox is in 'showing the "
	 "desktop' mode. In 'showing the desktop' mode, all normal clients "
	 "are hidden and the desktop is given focus if possible."),
    METH(setShowingDesktop,
	 "Enters or leaves 'showing the desktop' mode. See showingDesktop() "
	 "for a description of this mode."),
    METH(screenArea,
	 "Returns the on-screen available area. This is the area not reserved "
	 "by applications' struts. Windows should be placed within this area, "
	 "not within the physicalSize()."),
    METH(screenStrut,
	 "Returns the combined strut which has been reserved by all "
	 "applications on the desktops."),
    METH(physicalSize,
	 "Returns the physical size of the display device (in pixels)."),
    METH(screenNumber,
	 "Returns the number of the screen on which Openbox is running."),
    METH(rootWindow,
	 "Return the window id of the root window."),
    METH(clientList,
	 "Returns a all clients currently being managed by Openbox. This list "
	 "is updated as clients are managed and closed/destroyed/released."),
    { NULL, NULL, 0, NULL }
};

/***************************************************************************
 
   Type methods/struct
 
 ***************************************************************************/

/*static PyObject *owrap_getattr(OpenboxWrap *self, char *name)
{
    CHECK_OWRAP(self, "getattr");
    return Py_FindMethod(OpenboxWrapAttributeMethods, (PyObject*)self, name);
}*/

static void owrap_dealloc(OpenboxWrap *self)
{
    PyObject_Del((PyObject*) self);
}

static PyTypeObject OpenboxWrapType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Openbox",
    sizeof(OpenboxWrap),
    0,
    (destructor) owrap_dealloc,     /*tp_dealloc*/
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

/***************************************************************************
 
   Define the type 'OpenboxState'

 ***************************************************************************/

#define IS_OSTATE(v)  ((v)->ob_type == &OpenboxStateType)
#define CHECK_OSTATE(self, funcname) { \
    if (!IS_OSTATE(self)) { \
        PyErr_SetString(PyExc_TypeError, \
			"descriptor '" funcname "' requires a 'State' " \
			"object"); \
	return NULL; \
    } \
}

staticforward PyTypeObject OpenboxStateType;

typedef struct OpenboxState {
    PyObject_HEAD
} OpenboxState;

static void ostate_dealloc(PyObject *self)
{
    PyObject_Del(self);
}

static PyObject *ostate_getattr(OpenboxState *self, char *name)
{
    struct S { char *name; int val; };
    struct S s[] = {
	{ "Starting", State_Starting },
	{ "Running", State_Running },
	{ "Exiting", State_Exiting },
	{ NULL, 0 } };
    int i;

    CHECK_OSTATE(self, "__getattr__");

    for (i = 0; s[i].name != NULL; ++i)
	if (!strcmp(s[i].name, name))
	    return PyInt_FromLong(s[i].val);
    PyErr_SetString(PyExc_AttributeError, "invalid attribute");
    return NULL;
}

static PyTypeObject OpenboxStateType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "State",
    sizeof(OpenboxState),
    0,
    (destructor) ostate_dealloc,    /*tp_dealloc*/
    0,                              /*tp_print*/
    (getattrfunc) ostate_getattr,   /*tp_getattr*/
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

void openboxwrap_startup()
{
    PyObject *ob, *obdict, *state;

    OpenboxWrapType.ob_type = &PyType_Type;
    OpenboxWrapType.tp_methods = OpenboxWrapMethods;
    PyType_Ready(&OpenboxWrapType);

    /* get the ob module/dict */
    ob = PyImport_ImportModule("ob"); /* new */
    g_assert(ob != NULL);
    obdict = PyModule_GetDict(ob); /* borrowed */
    g_assert(obdict != NULL);

    /* add an Openbox instance to the ob module */
    openboxwrap_obj = PyObject_New(OpenboxWrap, &OpenboxWrapType);
    openboxwrap_obj->client_list = PyList_New(0);

    PyDict_SetItemString(obdict, "Openbox", (PyObject*) openboxwrap_obj);

    /* add an instance of OpenboxState */
    state = (PyObject*) PyObject_New(OpenboxState, &OpenboxStateType);
    PyDict_SetItemString(obdict, "State", state);
    Py_DECREF(state);

    Py_DECREF(ob);
}

void openboxwrap_shutdown()
{
    Py_DECREF(openboxwrap_obj->client_list);
    Py_DECREF(openboxwrap_obj);
}
