#include "pointer.h"
#include "keyboard.h"
#include "frame.h"
#include "engine.h"
#include "openbox.h"
#include "hooks.h"
#include "configwrap.h"

#include <glib.h>
#include <Python.h>
#include <structmember.h> /* for PyMemberDef stuff */
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

typedef enum {
    Action_Press,
    Action_Release,
    Action_Click,
    Action_DoubleClick,
    Action_Motion,
    NUM_ACTIONS
} Action;

/* GData of GSList*s of PointerBinding*s. */
static GData *bound_contexts;
static gboolean grabbed;
PyObject *grab_func;

struct foreach_grab_temp {
    Client *client;
    gboolean grab;
};

typedef struct {
    guint state;
    guint button;
    Action action;
    char *name;
    GSList *funcs[NUM_ACTIONS];
} PointerBinding;

/***************************************************************************
 
   Define the type 'ButtonData'

 ***************************************************************************/

typedef struct PointerData {
    PyObject_HEAD
    Action action;
    GQuark context;
    char *button;
    guint state;
    guint buttonnum;
    int posx, posy;
    int pressposx, pressposy;
    int pcareax, pcareay, pcareaw, pcareah;
} PointerData;

staticforward PyTypeObject PointerDataType;

/***************************************************************************
 
   Type methods/struct
 
 ***************************************************************************/

static PyObject *ptrdata_new(char *button, GQuark context, Action action,
			     guint state, guint buttonnum, int posx, int posy,
			     int pressposx, int pressposy, int pcareax,
			     int pcareay, int pcareaw, int pcareah)
{
    PointerData *self = PyObject_New(PointerData, &PointerDataType);
    self->button = g_strdup(button);
    self->context = context;
    self->action = action;
    self->state = state;
    self->buttonnum = buttonnum;
    self->posx = posx;
    self->posy = posy;
    self->pressposx = pressposx;
    self->pressposy = pressposy;
    self->pcareax = pcareax;
    self->pcareay = pcareay;
    self->pcareaw = pcareaw;
    self->pcareah = pcareah;
    return (PyObject*) self;
}

static void ptrdata_dealloc(PointerData *self)
{
    g_free(self->button);
    PyObject_Del((PyObject*)self);
}

static PyObject *ptrdata_getattr(PointerData *self, char *name)
{
    if (!strcmp(name, "button"))
	return PyString_FromString(self->button);
    if (!strcmp(name, "action"))
	return PyInt_FromLong(self->action);
    if (!strcmp(name, "context"))
	return PyString_FromString(g_quark_to_string(self->context));
    if (!strcmp(name, "state"))
	return PyInt_FromLong(self->state);
    if (!strcmp(name, "buttonnum"))
	return PyInt_FromLong(self->buttonnum);

    if (self->action == Action_Motion) { /* the rest are only for motions */
	if (!strcmp(name, "pos")) {
	    PyObject *pos = PyTuple_New(2);
	    PyTuple_SET_ITEM(pos, 0, PyInt_FromLong(self->posx));
	    PyTuple_SET_ITEM(pos, 1, PyInt_FromLong(self->posy));
	    return pos;
	}
	if (!strcmp(name, "presspos")) {
	    PyObject *presspos = PyTuple_New(2);
	    PyTuple_SET_ITEM(presspos, 0, PyInt_FromLong(self->pressposx));
	    PyTuple_SET_ITEM(presspos, 1, PyInt_FromLong(self->pressposy));
	    return presspos;
	}
	if (!strcmp(name, "pressclientarea")) {
	    if (self->pcareaw < 0) { /* < 0 indicates no client */
		Py_INCREF(Py_None);
		return Py_None;
	    } else {
		PyObject *ca = PyTuple_New(4);
		PyTuple_SET_ITEM(ca, 0, PyInt_FromLong(self->pcareax));
		PyTuple_SET_ITEM(ca, 1, PyInt_FromLong(self->pcareay));
		PyTuple_SET_ITEM(ca, 2, PyInt_FromLong(self->pcareaw));
		PyTuple_SET_ITEM(ca, 3, PyInt_FromLong(self->pcareah));
		return ca;
	    }
	}
    }

    PyErr_Format(PyExc_AttributeError, "no such attribute '%s'", name);
    return NULL;
}

static PyTypeObject PointerDataType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "PointerData",
    sizeof(PointerData),
    0,
    (destructor) ptrdata_dealloc,   /*tp_dealloc*/
    0,                              /*tp_print*/
    (getattrfunc) ptrdata_getattr,  /*tp_getattr*/
    0,                              /*tp_setattr*/
    0,                              /*tp_compare*/
    0,                              /*tp_repr*/
    0,                              /*tp_as_number*/
    0,                              /*tp_as_sequence*/
    0,                              /*tp_as_mapping*/
    0,                              /*tp_hash */
};

/***************************************************************************/

static gboolean translate(char *str, guint *state, guint *button)
{
    char **parsed;
    char *l;
    int i;
    gboolean ret = FALSE;

    parsed = g_strsplit(str, "-", -1);
    
    /* first, find the button (last token) */
    l = NULL;
    for (i = 0; parsed[i] != NULL; ++i)
	l = parsed[i];
    if (l == NULL)
	goto translation_fail;

    /* figure out the mod mask */
    *state = 0;
    for (i = 0; parsed[i] != l; ++i) {
	guint m = keyboard_translate_modifier(parsed[i]);
	if (!m) goto translation_fail;
	*state |= m;
    }

    /* figure out the button */
    *button = atoi(l);
    if (!*button) {
	g_warning("Invalid button '%s' in pointer binding.", l);
	goto translation_fail;
    }

    ret = TRUE;

translation_fail:
    g_strfreev(parsed);
    return ret;
}

static void grab_button(Client *client, guint state, guint button,
			GQuark context, gboolean grab)
{
    Window win;
    int mode = GrabModeAsync;
    unsigned int mask;

    if (context == g_quark_try_string("frame")) {
	win = client->frame->window;
	mask = ButtonPressMask | ButtonMotionMask | ButtonReleaseMask;
    } else if (context == g_quark_try_string("client")) {
	win = client->frame->plate;
	mode = GrabModeSync; /* this is handled in pointer_event */
	mask = ButtonPressMask; /* can't catch more than this with Sync mode
				   the release event is manufactured in
				   pointer_fire */
    } else return;

    if (grab)
	XGrabButton(ob_display, button, state, win, FALSE, mask, mode,
		    GrabModeAsync, None, None);
    else
	XUngrabButton(ob_display, button, state, win);
}

static void foreach_grab(GQuark key, gpointer data, gpointer user_data)
{
    struct foreach_grab_temp *d = user_data;
    GSList *it;
    for (it = data; it != NULL; it = it->next) {
	PointerBinding *b = it->data;
 	grab_button(d->client, b->state, b->button, key, d->grab);
    }
}
  
void pointer_grab_all(Client *client, gboolean grab)
{
    struct foreach_grab_temp bt;
    bt.client = client;
    bt.grab = grab;
    g_datalist_foreach(&bound_contexts, foreach_grab, &bt);
}

static void grab_all_clients(gboolean grab)
{
    GSList *it;

    for (it = client_list; it != NULL; it = it->next)
	pointer_grab_all(it->data, grab);
}

static gboolean grab_pointer(gboolean grab)
{
    gboolean ret = TRUE;
    if (grab)
	ret = XGrabPointer(ob_display, ob_root, FALSE, (ButtonPressMask |
							ButtonReleaseMask |
							ButtonMotionMask |
							PointerMotionMask),
			   GrabModeAsync, GrabModeAsync, None, None,
			   CurrentTime) == GrabSuccess;
    else
	XUngrabPointer(ob_display, CurrentTime);
    if (ret) grabbed = grab;
    return ret;
}

static void foreach_clear(GQuark key, gpointer data, gpointer user_data)
{
    GSList *it;
    user_data = user_data;
    for (it = data; it != NULL; it = it->next) {
	int i;

        PointerBinding *b = it->data;
	for (i = 0; i < NUM_ACTIONS; ++i)
	    while (b->funcs[i] != NULL) {
		Py_DECREF((PyObject*)b->funcs[i]->data);
		b->funcs[i] = g_slist_delete_link(b->funcs[i], b->funcs[i]);
	    }
        g_free(b->name);
        g_free(b);
    }
    g_slist_free(data);
}

static void clearall()
{
    grab_all_clients(FALSE);
    g_datalist_foreach(&bound_contexts, foreach_clear, NULL);
}

static void fire_event(char *button, GQuark context, Action action,
                       guint state, guint buttonnum, int posx, int posy,
                       int pressposx, int pressposy, int pcareax,
                       int pcareay, int pcareaw, int pcareah,
                       PyObject *client, GSList *functions)
{
    PyObject *ptrdata, *args, *ret;
    GSList *it;

    ptrdata = ptrdata_new(button, context, action,
                          state, buttonnum, posx, posy, pressposx, pressposy,
                          pcareax, pcareay, pcareaw, pcareah);
    args = Py_BuildValue("OO", ptrdata, client);

    if (grabbed) {
        ret = PyObject_CallObject(grab_func, args);
        if (ret == NULL) PyErr_Print();
        Py_XDECREF(ret);
    } else {
        for (it = functions; it != NULL; it = it->next) {
            ret = PyObject_CallObject(it->data, args);
            if (ret == NULL) PyErr_Print();
            Py_XDECREF(ret);
        }
    }

    Py_DECREF(args);
    Py_DECREF(ptrdata);
}

void pointer_event(XEvent *e, Client *c)
{
    static guint button = 0, lastbutton = 0;
    static Time time = 0;
    static Rect carea;
    static guint pressx, pressy;
    GQuark contextq;
    gboolean click = FALSE, dblclick = FALSE;
    PyObject *client;
    GString *str = g_string_sized_new(0);
    guint state;
    GSList *it = NULL;
    PointerBinding *b = NULL;
    guint drag_threshold;

    drag_threshold = configwrap_get_int("input", "drag_threshold");

    contextq = engine_get_context(c, e->xany.window);

    /* pick a button, figure out clicks/double clicks */
    switch (e->type) {
    case ButtonPress:
	if (!button) {
	    button = e->xbutton.button;
	    if (c != NULL) carea = c->frame->area;
	    else carea.width = -1; /* indicates no client */
	    pressx = e->xbutton.x_root;
	    pressy = e->xbutton.y_root;
	}
	state = e->xbutton.state;
	break;
    case ButtonRelease:
	state = e->xbutton.state;
	break;
    case MotionNotify:
	state = e->xmotion.state;
	break;
    default:
	g_assert_not_reached();
	return;
    }

    if (!grabbed) {
	for (it = g_datalist_id_get_data(&bound_contexts, contextq);
	     it != NULL; it = it->next) {
	    b = it->data;
	    if (b->state == state && b->button == button)
		break;
	}
        /* if not grabbed and not bound, then nothing to do! */
        if (it == NULL) return;
    }

    if (c) client = clientwrap_new(c);
    else client = Py_None;

    /* build the button string */
    if (state & ControlMask) g_string_append(str, "C-");
    if (state & ShiftMask)   g_string_append(str, "S-");
    if (state & Mod1Mask)    g_string_append(str, "Mod1-");
    if (state & Mod2Mask)    g_string_append(str, "Mod2-");
    if (state & Mod3Mask)    g_string_append(str, "Mod3-");
    if (state & Mod4Mask)    g_string_append(str, "Mod4-");
    if (state & Mod5Mask)    g_string_append(str, "Mod5-");
    g_string_append_printf(str, "%d", button);

    /* figure out clicks/double clicks */
    switch (e->type) {
    case ButtonRelease:
	if (button == e->xbutton.button) {
	    /* determine if this is a valid 'click'. Its not if the release is
	     not over the window, or if a drag occured. */
	    if (ABS(e->xbutton.x_root - pressx) < drag_threshold &&
		ABS(e->xbutton.y_root - pressy) < drag_threshold &&
		e->xbutton.x >= 0 && e->xbutton.y >= 0) {
		int junk;
		Window wjunk;
		guint ujunk, w, h;
		XGetGeometry(ob_display, e->xany.window, &wjunk, &junk, &junk,
			     &w, &h, &ujunk, &ujunk);
		if (e->xbutton.x < (signed)w && e->xbutton.y < (signed)h)
		    click =TRUE;
	    }

	    /* determine if this is a valid 'double-click' */
	    if (click) {
		if (lastbutton == button &&
		    e->xbutton.time -
                    configwrap_get_int("input", "double_click_rate") < time) {
		    dblclick = TRUE;
		    lastbutton = 0;
		} else
		    lastbutton = button;
	    } else
		lastbutton = 0;
	    time = e->xbutton.time;
	    pressx = pressy = 0;
	    button = 0;
	    carea.x = carea.y = carea.width = carea.height = 0;
	}
	break;
    }

    /* fire off the events */
    switch (e->type) {
    case ButtonPress:
        fire_event(str->str, contextq, Action_Press,
                   state, button, 0, 0, 0, 0, 0, 0, 0, 0,
                   client, b == NULL ? NULL : b->funcs[Action_Press]);
        break;
    case ButtonRelease:
        fire_event(str->str, contextq, Action_Release,
                   state, button, 0, 0, 0, 0, 0, 0, 0, 0,
                   client, b == NULL ? NULL : b->funcs[Action_Release]);
        break;
    case MotionNotify:
        /* watch out for the drag threshold */
        if (ABS(e->xmotion.x_root - pressx) < drag_threshold &&
            ABS(e->xmotion.y_root - pressy) < drag_threshold)
            break;
        fire_event(str->str, contextq, Action_Motion,
                   state, button, e->xmotion.x_root,
                   e->xmotion.y_root, pressx, pressy,
                   carea.x, carea.y, carea.width, carea.height,
                   client, b == NULL ? NULL : b->funcs[Action_Motion]);
        break;
    }

    if (click)
        fire_event(str->str, contextq, Action_Click,
                   state, button, 0, 0, 0, 0, 0, 0, 0, 0,
                   client, b == NULL ? NULL : b->funcs[Action_Click]);
    if (dblclick)
        fire_event(str->str, contextq, Action_DoubleClick,
                   state, button, 0, 0, 0, 0, 0, 0, 0, 0,
                   client, b == NULL ? NULL : b->funcs[Action_DoubleClick]);

    g_string_free(str, TRUE);
    if (client != Py_None) { Py_DECREF(client); }

    if (contextq == g_quark_try_string("client")) {
        /* Replay the event, so it goes to the client*/
        XAllowEvents(ob_display, ReplayPointer, CurrentTime);
        /* generate a release event since we don't get real ones */
        if (e->type == ButtonPress) {
            e->type = ButtonRelease;
            pointer_event(e, c);
        }
    }
}

/***************************************************************************
 
   Define the type 'Pointer'

 ***************************************************************************/

#define IS_POINTER(v)  ((v)->ob_type == &PointerType)
#define CHECK_POINTER(self, funcname) { \
    if (!IS_POINTER(self)) { \
        PyErr_SetString(PyExc_TypeError, \
			"descriptor '" funcname "' requires a 'Pointer' " \
			"object"); \
	return NULL; \
    } \
}

typedef struct Pointer {
    PyObject_HEAD
    Action press;
    Action release;
    Action click;
    Action doubleclick;
    Action motion;
} Pointer;

staticforward PyTypeObject PointerType;

static PyObject *ptr_bind(Pointer *self, PyObject *args)
{
    char *buttonstr;
    char *contextstr;
    guint state, button;
    PointerBinding *b;
    GSList *it;
    GQuark context;
    PyObject *func;
    Action action;
    int i;

    CHECK_POINTER(self, "grab");
    if (!PyArg_ParseTuple(args, "ssiO:grab",
			  &buttonstr, &contextstr, &action, &func))
	return NULL;

    if (!translate(buttonstr, &state, &button)) {
	PyErr_SetString(PyExc_ValueError, "invalid button");
	return NULL;
    }

    context = g_quark_try_string(contextstr);
    if (!context) {
	PyErr_SetString(PyExc_ValueError, "invalid context");
	return NULL;
    }

    if (action < 0 || action >= NUM_ACTIONS) {
	PyErr_SetString(PyExc_ValueError, "invalid action");
	return NULL;
    }

    if (!PyCallable_Check(func)) {
	PyErr_SetString(PyExc_ValueError, "expected a callable object");
	return NULL;
    }

    for (it = g_datalist_id_get_data(&bound_contexts, context);
	 it != NULL; it = it->next){
	b = it->data;
	if (b->state == state && b->button == button) {
	    /* already bound */
	    b->funcs[action] = g_slist_append(b->funcs[action], func);
	    Py_INCREF(Py_None);
	    return Py_None;
	}
    }

    grab_all_clients(FALSE);

    /* add the binding */
    b = g_new(PointerBinding, 1);
    b->state = state;
    b->button = button;
    b->name = g_strdup(buttonstr);
    for (i = 0; i < NUM_ACTIONS; ++i)
	if (i != (signed)action) b->funcs[i] = NULL;
    b->funcs[action] = g_slist_append(NULL, func);
    g_datalist_id_set_data(&bound_contexts, context, 
        g_slist_append(g_datalist_id_get_data(&bound_contexts, context), b));
    grab_all_clients(TRUE);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *ptr_clearBinds(Pointer *self, PyObject *args)
{
    CHECK_POINTER(self, "clearBinds");
    if (!PyArg_ParseTuple(args, ":clearBinds"))
	return NULL;
    clearall();
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *ptr_grab(Pointer *self, PyObject *args)
{
    PyObject *func;

    CHECK_POINTER(self, "grab");
    if (!PyArg_ParseTuple(args, "O:grab", &func))
	return NULL;
    if (!PyCallable_Check(func)) {
	PyErr_SetString(PyExc_ValueError, "expected a callable object");
	return NULL;
    }
    if (!grab_pointer(TRUE)) {
	PyErr_SetString(PyExc_RuntimeError, "failed to grab pointer");
	return NULL;
    }
    grab_func = func;
    Py_INCREF(grab_func);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *ptr_ungrab(Pointer *self, PyObject *args)
{
    CHECK_POINTER(self, "ungrab");
    if (!PyArg_ParseTuple(args, ":ungrab"))
	return NULL;
    grab_pointer(FALSE);
    Py_XDECREF(grab_func);
    grab_func = NULL;
    Py_INCREF(Py_None);
    return Py_None;
}

#define METH(n, d) {#n, (PyCFunction)ptr_##n, METH_VARARGS, #d}

static PyMethodDef PointerMethods[] = {
    METH(bind,
	 "bind(button, context, func)\n\n"
	 "Binds a pointer button for a context to a function. See the "
	 "Terminology section for a decription and list of common contexts. "
	 "The button is a string which defines a modifier and button "
	 "combination with the format [Modifier-]...[Button]. Modifiers can "
	 "be 'mod1', 'mod2', 'mod3', 'mod4', 'mod5', 'control', and 'shift'. "
	 "The keys on your keyboard that are bound to each of these modifiers "
	 "can be found by running 'xmodmap'. The button is the number of the "
	 "button. Button numbers can be found by running 'xev', pressing the "
	 "button with the pointer over its window, and watching its output. "
	 "Here are some examples of valid buttons: 'control-1', '2', "
	 "'mod1-shift-5'. The func must have a definition similar to "
	 "'def func(keydata, client)'. A button and context may be bound to "
	 "more than one function."),
    METH(clearBinds,
	 "clearBinds()\n\n"
	 "Removes all bindings that were previously made by bind()."),
    METH(grab,
	 "grab(func)\n\n"
	 "Grabs the pointer device, causing all possible pointer events to be "
	 "sent to the given function. CAUTION: Be sure when you grab() that "
	 "you also have an ungrab() that will execute, or you will not be "
	 "able to use the pointer device until you restart Openbox. The func "
	 "must have a definition similar to 'def func(keydata)'. The pointer "
	 "cannot be grabbed if it is already grabbed."),
    METH(ungrab,
	 "ungrab()\n\n"
	 "Ungrabs the pointer. The pointer cannot be ungrabbed if it is not "
	 "grabbed."),
    { NULL, NULL, 0, NULL }
};

static PyMemberDef PointerMembers[] = {
    {"Action_Press", T_INT, offsetof(Pointer, press), READONLY,
     "a pointer button press"},
    {"Action_Release", T_INT, offsetof(Pointer, release), READONLY,
     "a pointer button release"},
    {"Action_Click", T_INT, offsetof(Pointer, click), READONLY,
     "a pointer button click (press-release)"},
    {"Action_DoubleClick", T_INT, offsetof(Pointer, doubleclick), READONLY,
     "a pointer button double-click"},
    {"Action_Motion", T_INT, offsetof(Pointer, motion), READONLY,
     "a pointer drag"},
    {NULL}
};

/***************************************************************************
 
   Type methods/struct
 
 ***************************************************************************/

static void ptr_dealloc(PyObject *self)
{
    PyObject_Del(self);
}

static PyTypeObject PointerType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Pointer",
    sizeof(Pointer),
    0,
    (destructor) ptr_dealloc,       /*tp_dealloc*/
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

void pointer_startup()
{
    PyObject *input, *inputdict;
    Pointer *ptr;

    grabbed = FALSE;
    configwrap_add_int("input", "double_click_rate", "Double-Click Rate",
                       "An integer containing the number of milliseconds in "
                       "which 2 clicks must be received to cause a "
                       "double-click event.", 300);
    configwrap_add_int("input", "drag_threshold", "Drag Threshold",
                       "An integer containing the number of pixels a drag "
                       "must go before motion events start getting generated. "
                       "Once a drag has begun, the button release will not "
                       "count as a click event.", 3);
    g_datalist_init(&bound_contexts);

    PointerType.ob_type = &PyType_Type;
    PointerType.tp_methods = PointerMethods;
    PointerType.tp_members = PointerMembers;
    PyType_Ready(&PointerType);
    PyType_Ready(&PointerDataType);

    /* get the input module/dict */
    input = PyImport_ImportModule("input"); /* new */
    g_assert(input != NULL);
    inputdict = PyModule_GetDict(input); /* borrowed */
    g_assert(inputdict != NULL);

    /* add a Pointer instance to the input module */
    ptr = PyObject_New(Pointer, &PointerType);
    ptr->press = Action_Press;
    ptr->release = Action_Release;
    ptr->click = Action_Click;
    ptr->doubleclick = Action_DoubleClick;
    ptr->motion = Action_Motion;
    PyDict_SetItemString(inputdict, "Pointer", (PyObject*) ptr);
    Py_DECREF(ptr);

    Py_DECREF(input);
}

void pointer_shutdown()
{
    if (grabbed)
	grab_pointer(FALSE);
    clearall();
    g_datalist_clear(&bound_contexts);
}

