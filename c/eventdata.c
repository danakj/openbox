#include "eventdata.h"
#include "openbox.h"
#include "event.h"
#include "clientwrap.h"
#include <X11/Xlib.h>

/*
 *
 * Define the type 'EventData'
 *
 */

#define IS_EVENTDATA(v)  ((v)->ob_type == &EventDataType)
#define CHECK_EVENTDATA(self, funcname) { \
    if (!IS_EVENTDATA(self)) { \
        PyErr_SetString(PyExc_TypeError, \
			"descriptor '" funcname "' requires an 'EventData' " \
			"object"); \
	return NULL; \
    } \
}

staticforward PyTypeObject EventDataType;

static PyObject *eventdata_type(EventData *self, PyObject *args)
{
    CHECK_EVENTDATA(self, "type");
    if (!PyArg_ParseTuple(args, ":type"))
	return NULL;
    return PyInt_FromLong(self->type);
}

static PyObject *eventdata_time(EventData *self, PyObject *args)
{
    CHECK_EVENTDATA(self, "time");
    if (!PyArg_ParseTuple(args, ":time"))
	return NULL;
    return PyInt_FromLong(event_lasttime);
}

static PyObject *eventdata_context(EventData *self, PyObject *args)
{
    CHECK_EVENTDATA(self, "context");
    if (!PyArg_ParseTuple(args, ":context"))
	return NULL;
    return PyString_FromString(self->context);
}

static PyObject *eventdata_client(EventData *self, PyObject *args)
{
    CHECK_EVENTDATA(self, "client");
    if (!PyArg_ParseTuple(args, ":client"))
	return NULL;
    if (self->client == NULL) {
	Py_INCREF(Py_None);
	return Py_None;
    } else {
	return clientwrap_new(self->client);
    }
}

static PyObject *eventdata_keycode(EventData *self, PyObject *args)
{
    CHECK_EVENTDATA(self, "keycode");
    if (!PyArg_ParseTuple(args, ":keycode"))
	return NULL;
    switch (self->type) {
    case Key_Press:
    case Key_Release:
	break;
    default:
	PyErr_SetString(PyExc_TypeError,
			"The EventData object is not a Key event");
	return NULL;
    }
    return PyInt_FromLong(self->details.key->keycode);
}

static PyObject *eventdata_modifiers(EventData *self, PyObject *args)
{
    CHECK_EVENTDATA(self, "key");
    if (!PyArg_ParseTuple(args, ":key"))
	return NULL;
    switch (self->type) {
    case Key_Press:
    case Key_Release:
    case Pointer_Press:
    case Pointer_Release:
    case Pointer_Motion:
	break;
    default:
	PyErr_SetString(PyExc_TypeError,
			"The EventData object is not a Key or Pointer event");
	return NULL;
    }
    return PyInt_FromLong(self->details.key->modifiers);
}

static PyObject *eventdata_keyName(EventData *self, PyObject *args)
{
    GList *it;
    PyObject *tuple;
    int i;

    CHECK_EVENTDATA(self, "keyName");
    if (!PyArg_ParseTuple(args, ":keyName"))
	return NULL;
    switch (self->type) {
    case Key_Press:
    case Key_Release:
	break;
    default:
	PyErr_SetString(PyExc_TypeError,
			"The EventData object is not a Key event");
	return NULL;
    }

    if (self->details.key->keylist != NULL) {
	tuple = PyTuple_New(g_list_length(self->details.key->keylist));
	for (i = 0, it = self->details.key->keylist; it != NULL;
	     it = it->next, ++i)
	    PyTuple_SET_ITEM(tuple, i, PyString_FromString(it->data));
	return tuple;
    } else {
	GString *str = g_string_sized_new(0);
	KeySym sym;

	if (self->details.key->modifiers & ControlMask)
	    g_string_append(str, "C-");
	if (self->details.key->modifiers & ShiftMask)
	    g_string_append(str, "S-");
	if (self->details.key->modifiers & Mod1Mask)
	    g_string_append(str, "Mod1-");
	if (self->details.key->modifiers & Mod2Mask)
	    g_string_append(str, "Mod2-");
	if (self->details.key->modifiers & Mod3Mask)
	    g_string_append(str, "Mod3-");
	if (self->details.key->modifiers & Mod4Mask)
	    g_string_append(str, "Mod4-");
	if (self->details.key->modifiers & Mod5Mask)
	    g_string_append(str, "Mod5-");

	sym = XKeycodeToKeysym(ob_display, self->details.key->keycode, 0);
	if (sym == NoSymbol)
	    g_string_append(str, "NoSymbol");
	else {
	    char *name = XKeysymToString(sym);
	    if (name == NULL)
		name = "Undefined";
	    g_string_append(str, name);
	}

	tuple = PyTuple_New(1);
	PyTuple_SET_ITEM(tuple, 0, PyString_FromString(str->str));
	g_string_free(str, TRUE);

	return tuple;
    }
}

static PyObject *eventdata_button(EventData *self, PyObject *args)
{
    CHECK_EVENTDATA(self, "button");
    if (!PyArg_ParseTuple(args, ":button"))
	return NULL;
    switch (self->type) {
    case Pointer_Press:
    case Pointer_Release:
    case Pointer_Motion:
	break;
    default:
	PyErr_SetString(PyExc_TypeError,
			"The EventData object is not a Pointer event");
	return NULL;
    }
    return PyInt_FromLong(self->details.pointer->button);
}

static PyObject *eventdata_buttonName(EventData *self, PyObject *args)
{
    CHECK_EVENTDATA(self, "buttonName");
    if (!PyArg_ParseTuple(args, ":buttonName"))
	return NULL;
    switch (self->type) {
    case Pointer_Press:
    case Pointer_Release:
    case Pointer_Motion:
	break;
    default:
	PyErr_SetString(PyExc_TypeError,
			"The EventData object is not a Pointer event");
	return NULL;
    }

    if (self->details.pointer->name != NULL) {
	return PyString_FromString(self->details.pointer->name);
    } else {
	PyObject *pystr;
	GString *str = g_string_sized_new(0);

	if (self->details.pointer->modifiers & ControlMask)
	    g_string_append(str, "C-");
	if (self->details.pointer->modifiers & ShiftMask)
	    g_string_append(str, "S-");
	if (self->details.pointer->modifiers & Mod1Mask)
	    g_string_append(str, "Mod1-");
	if (self->details.pointer->modifiers & Mod2Mask)
	    g_string_append(str, "Mod2-");
	if (self->details.pointer->modifiers & Mod3Mask)
	    g_string_append(str, "Mod3-");
	if (self->details.pointer->modifiers & Mod4Mask)
	    g_string_append(str, "Mod4-");
	if (self->details.pointer->modifiers & Mod5Mask)
	    g_string_append(str, "Mod5-");
	
	g_string_append_printf(str, "%d", self->details.pointer->button);

	pystr = PyString_FromString(str->str);

	g_string_free(str, TRUE);

	return pystr;
    }
}

static PyObject *eventdata_position(EventData *self, PyObject *args)
{
    PyObject *tuple;

    CHECK_EVENTDATA(self, "position");
    if (!PyArg_ParseTuple(args, ":position"))
	return NULL;
    switch (self->type) {
    case Pointer_Press:
    case Pointer_Release:
    case Pointer_Motion:
	break;
    default:
	PyErr_SetString(PyExc_TypeError,
			"The EventData object is not a Pointer event");
	return NULL;
    }
    tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(self->details.pointer->xroot));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(self->details.pointer->yroot));
    return tuple;
}

static PyMethodDef EventDataAttributeMethods[] = {
    {"type", (PyCFunction)eventdata_type, METH_VARARGS,
     "data.type() -- Return the event type"},
    {"context", (PyCFunction)eventdata_context, METH_VARARGS,
     "data.context() -- Return the context for the event. If it is "
     "\"client\", then data.client() can be used to find out the "
     "client."},
    {"client", (PyCFunction)eventdata_client, METH_VARARGS,
     "data.client() -- Return the client for the event. This may be None if "
     "there is no client, even if data.context() gives Context_Client."},
    {"time", (PyCFunction)eventdata_time, METH_VARARGS,
     "data.time() -- Return the time at which the last X event occured with "
     "a timestamp. Should be the time at which this event, or the event that "
     "caused this event to occur happened."},
    {"modifiers", (PyCFunction)eventdata_modifiers, METH_VARARGS,
     "data.modifiers() -- Return the modifier keymask that was pressed "
     "when the event occured. A bitmask of ShiftMask, LockMask, ControlMask, "
     "Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, and Mod5Mask. Cannot be used "
     "when the data.type() is not a Key_* or Pointer_* event type."},
    {"keycode", (PyCFunction)eventdata_keycode, METH_VARARGS,
     "data.keycode() -- Return the keycode for the key which generated the "
     "event. Cannot be used when the data.type() is not a Key_* event type."},
    {"keyName", (PyCFunction)eventdata_keyName, METH_VARARGS,
     "data.keyName() -- Return a tuple of the string names of the key which "
     "generated the event. Cannot be used when the data.type() is not a Key_* "
     "event "
     "type."},
    {"button", (PyCFunction)eventdata_button, METH_VARARGS,
     "data.button() -- Return the pointer button which generated the event. "
     "Cannot be used when the data.type() is not a Pointer_* event type."},
    {"buttonName", (PyCFunction)eventdata_keyName, METH_VARARGS,
     "data.buttonName() -- Return the name of the button which generated the "
     "event. Cannot be used when the data.type() is not a Pointer_* event "
     "type."},
    {"position", (PyCFunction)eventdata_position, METH_VARARGS,
     "data.position() -- Returns the current position of the pointer on the "
     "root window when the event was generated. Gives the position in a tuple "
     "with a format of (x, y). Cannot be used when the data.type() is not a "
     "Pointer_* event type."},
    { NULL, NULL, 0, NULL }
};

static void data_dealloc(EventData *self)
{
    GList *it;

    switch(self->type) {
    case Logical_EnterWindow:
    case Logical_LeaveWindow:
    case Logical_NewWindow:
    case Logical_CloseWindow:
    case Logical_Startup:
    case Logical_Shutdown:
    case Logical_RequestActivate:
    case Logical_WindowShow:
    case Logical_WindowHide:
    case Logical_Focus:
    case Logical_Bell:
    case Logical_UrgentWindow:
	g_free(self->details.logical);
	break;
    case Pointer_Press:
    case Pointer_Release:
    case Pointer_Motion:
	if (self->details.pointer->name != NULL)
	    g_free(self->details.pointer->name);
	g_free(self->details.pointer);
	break;
    case Key_Press:
    case Key_Release:
	for (it = self->details.key->keylist; it != NULL; it = it->next)
	    g_free(it->data);
	g_list_free(self->details.key->keylist);
	g_free(self->details.key);
	break;
    default:
	g_assert_not_reached();
    }
    PyObject_Del((PyObject*) self);
}

static PyObject *eventdata_getattr(EventData *self, char *name)
{
    return Py_FindMethod(EventDataAttributeMethods, (PyObject*)self, name);
}

static PyTypeObject EventDataType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "EventData",
    sizeof(EventData),
    0,
    (destructor) data_dealloc,       /*tp_dealloc*/
    0,                               /*tp_print*/
    (getattrfunc) eventdata_getattr, /*tp_getattr*/
    0,                               /*tp_setattr*/
    0,                               /*tp_compare*/
    0,                               /*tp_repr*/
    0,                               /*tp_as_number*/
    0,                               /*tp_as_sequence*/
    0,                               /*tp_as_mapping*/
    0,                               /*tp_hash */
};



void eventdata_startup()
{
    EventDataType.ob_type = &PyType_Type;
    PyType_Ready(&EventDataType);
}

void eventdata_shutdown()
{
}

void eventdata_free(EventData *data)
{
    Py_DECREF(data);
}

EventData *eventdata_new_logical(EventType type, GQuark context,
				 struct Client *client)
{
    EventData *data;

    g_assert(type < Pointer_Press);

    data = PyObject_New(EventData, &EventDataType);
    data->type = type;
    data->context = g_quark_to_string(context);
    data->client = client;
    data->details.logical = g_new(LogicalEvent, 1);
    return data;
}

EventData *eventdata_new_pointer(EventType type, GQuark context,
				 struct Client *client, guint modifiers,
				 guint button, char *name,
				 int xroot, int yroot)
{
    EventData *data;

    g_assert(type >= Pointer_Press && type < Key_Press);

    data = PyObject_New(EventData, &EventDataType);
    data->type = type;
    data->context = g_quark_to_string(context);
    data->client = client;
    data->details.pointer = g_new(PointerEvent, 1);
    data->details.pointer->modifiers = modifiers;
    data->details.pointer->button = button;
    data->details.pointer->name = name == NULL ? name : g_strdup(name);
    data->details.pointer->xroot = xroot;
    data->details.pointer->yroot = yroot;
    return data;
}

EventData *eventdata_new_key(EventType type, GQuark context,
			     struct Client *client, guint modifiers,
			     guint keycode, GList *keylist)
{
    EventData *data;
    GList *mykeylist, *it;

    g_assert(type >= Key_Press);

    data = PyObject_New(EventData, &EventDataType);
    data->type = type;
    data->context = g_quark_to_string(context);
    data->client = client;
    data->details.key = g_new(KeyEvent, 1);

    /* make a copy of the keylist.
     If the user were to clear the key bindings, then the keylist given here
     would no longer point at valid memory.*/
    mykeylist = g_list_copy(keylist); /* shallow copy */
    for (it = mykeylist; it != NULL; it = it->next) /* deep copy */
	it->data = g_strdup(it->data);

    data->details.key->keylist = mykeylist;
    data->details.key->keycode = keycode;
    data->details.key->modifiers = modifiers;
    return data;
}
