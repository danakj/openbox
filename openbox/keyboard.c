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

guint keyboard_translate_modifier(char *str)
{
    if (!strcmp("Mod1", str)) return Mod1Mask;
    else if (!strcmp("Mod2", str)) return Mod2Mask;
    else if (!strcmp("Mod3", str)) return Mod3Mask;
    else if (!strcmp("Mod4", str)) return Mod4Mask;
    else if (!strcmp("Mod5", str)) return Mod5Mask;
    else if (!strcmp("C", str)) return ControlMask;
    else if (!strcmp("S", str)) return ShiftMask;
    g_warning("Invalid modifier '%s' in binding.", str);
    return 0;
}

static gboolean translate(char *str, guint *state, guint *keycode)
{
    char **parsed;
    char *l;
    int i;
    gboolean ret = FALSE;
    KeySym sym;

    parsed = g_strsplit(str, "-", -1);
    
    /* first, find the key (last token) */
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

    /* figure out the keycode */
    sym = XStringToKeysym(l);
    if (sym == NoSymbol) {
	g_warning("Invalid key name '%s' in key binding.", l);
	goto translation_fail;
    }
    *keycode = XKeysymToKeycode(ob_display, sym);
    if (!keycode) {
	g_warning("Key '%s' does not exist on the display.", l); 
	goto translation_fail;
    }

    ret = TRUE;

translation_fail:
    g_strfreev(parsed);
    return ret;
}

static void destroytree(KeyBindingTree *tree)
{
    KeyBindingTree *c;

    while (tree) {
	destroytree(tree->next_sibling);
	c = tree->first_child;
	if (c == NULL) {
	    GList *it;
	    for (it = tree->keylist; it != NULL; it = it->next)
		g_free(it->data);
	    g_list_free(tree->keylist);
	    Py_XDECREF(tree->func);
	}
	g_free(tree);
	tree = c;
    }
}

static KeyBindingTree *buildtree(GList *keylist)
{
    GList *it;
    KeyBindingTree *ret = NULL, *p;

    if (g_list_length(keylist) <= 0)
	return NULL; /* nothing in the list.. */

    for (it = g_list_last(keylist); it != NULL; it = it->prev) {
	p = ret;
	ret = g_new(KeyBindingTree, 1);
	ret->next_sibling = NULL;
	ret->func = NULL;
	if (p == NULL) {
	    GList *it;

	    /* this is the first built node, the bottom node of the tree */
	    ret->keylist = g_list_copy(keylist); /* shallow copy */
	    for (it = ret->keylist; it != NULL; it = it->next) /* deep copy */
		it->data = g_strdup(it->data);
	}
	ret->first_child = p;
	if (!translate(it->data, &ret->state, &ret->key)) {
	    destroytree(ret);
	    return NULL;
	}
    }
    return ret;
}

static void assimilate(KeyBindingTree *node)
{
    KeyBindingTree *a, *b, *tmp, *last;

    if (firstnode == NULL) {
	/* there are no nodes at this level yet */
	firstnode = node;
    } else {
	a = firstnode;
	last = a;
	b = node;
	while (a) {
	    last = a;
	    if (!(a->state == b->state && a->key == b->key)) {
		a = a->next_sibling;
	    } else {
		tmp = b;
		b = b->first_child;
		g_free(tmp);
		a = a->first_child;
	    }
	}
	if (!(last->state == b->state && last->key == b->key))
	    last->next_sibling = b;
	else {
	    last->first_child = b->first_child;
	    g_free(b);
	}
    }
}

static KeyBindingTree *find(KeyBindingTree *search, gboolean *conflict)
{
    KeyBindingTree *a, *b;

    *conflict = FALSE;

    a = firstnode;
    b = search;
    while (a && b) {
	if (!(a->state == b->state && a->key == b->key)) {
	    a = a->next_sibling;
	} else {
	    if ((a->first_child == NULL) == (b->first_child == NULL)) {
		if (a->first_child == NULL) {
		    /* found it! (return the actual node, not the search's) */
		    return a;
		}
	    } else {
		*conflict = TRUE;
		return NULL; /* the chain status' don't match (conflict!) */
	    }
	    b = b->first_child;
	    a = a->first_child;
	}
    }
    return NULL; // it just isn't in here
}

static void grab_keys(gboolean grab)
{
    if (!grab) {
	XUngrabKey(ob_display, AnyKey, AnyModifier, ob_root);
    } else {
	KeyBindingTree *p = firstnode;
	while (p) {
	    XGrabKey(ob_display, p->key, p->state, ob_root, FALSE,
		     GrabModeAsync, GrabModeSync);
	    p = p->next_sibling;
	}
    }
}

static void reset_chains()
{
    /* XXX kill timer */
    curpos = NULL;
    if (grabbed) {
	grabbed = FALSE;
	g_message("reset chains. user_grabbed: %d", user_grabbed);
	if (!user_grabbed)
	    XUngrabKeyboard(ob_display, CurrentTime);
    }
}

void keyboard_event(XKeyEvent *e)
{
    PyObject *chain, *client, *args, *keybdata, *ret;
    gboolean press = e->type == KeyPress;

    if (focus_client) client = clientwrap_new(focus_client);
    else client = Py_None;

    if (user_grabbed) {
	GString *str = g_string_sized_new(0);
	KeySym sym;

	/* build the 'chain' */
	if (e->state & ControlMask)
	    g_string_append(str, "C-");
	if (e->state & ShiftMask)
	    g_string_append(str, "S-");
	if (e->state & Mod1Mask)
	    g_string_append(str, "Mod1-");
	if (e->state & Mod2Mask)
	    g_string_append(str, "Mod2-");
	if (e->state & Mod3Mask)
	    g_string_append(str, "Mod3-");
	if (e->state & Mod4Mask)
	    g_string_append(str, "Mod4-");
	if (e->state & Mod5Mask)
	    g_string_append(str, "Mod5-");

	sym = XKeycodeToKeysym(ob_display, e->keycode, 0);
	if (sym == NoSymbol)
	    g_string_append(str, "NoSymbol");
	else {
	    char *name = XKeysymToString(sym);
	    if (name == NULL)
		name = "Undefined";
	    g_string_append(str, name);
	}

	chain = PyTuple_New(1);
	PyTuple_SET_ITEM(chain, 0, PyString_FromString(str->str));
	g_string_free(str, TRUE);

	keybdata = keybdata_new(chain, e->state, e->keycode, press);

	args = Py_BuildValue("OO", keybdata, client);

	ret = PyObject_CallObject(grab_func, args);
	if (ret == NULL) PyErr_Print();
	Py_XDECREF(ret);

	Py_DECREF(args);
	Py_DECREF(keybdata);
	Py_DECREF(chain);
    }

    if (press) {
	if (e->keycode == reset_key && e->state == reset_state) {
	    reset_chains();
	    XAllowEvents(ob_display, AsyncKeyboard, CurrentTime);
	} else {
	    KeyBindingTree *p;
	    if (curpos == NULL)
		p = firstnode;
	    else
		p = curpos->first_child;
	    while (p) {
		if (p->key == e->keycode && p->state == e->state) {
		    if (p->first_child != NULL) { /* part of a chain */
			/* XXX TIMER */
			if (!grabbed && !user_grabbed) {
			    /*grab should never fail because we should have a
			      sync grab at this point */
			    XGrabKeyboard(ob_display, ob_root, 0,
					  GrabModeAsync, GrabModeSync,
					  CurrentTime);
			}
			grabbed = TRUE;
			curpos = p;
			XAllowEvents(ob_display, AsyncKeyboard, CurrentTime);
		    } else {
			GList *it;
			int i;

			chain = PyTuple_New(g_list_length(p->keylist));
			for (i = 0, it = p->keylist; it != NULL;
			     it = it->next, ++i)
			    PyTuple_SET_ITEM(chain, i,
					     PyString_FromString(it->data));

			keybdata = keybdata_new(chain, e->state, e->keycode,
						press);

			args = Py_BuildValue("OO", keybdata, client);

			ret = PyObject_CallObject(p->func, args);
			if (ret == NULL) PyErr_Print();
			Py_XDECREF(ret);

			Py_DECREF(args);
			Py_DECREF(keybdata);
			Py_DECREF(chain);

			XAllowEvents(ob_display, AsyncKeyboard, CurrentTime);
			reset_chains();
		    }
		    break;
		}
		p = p->next_sibling;
	    }
	}
    }

    if (client != Py_None) { Py_DECREF(client); }
}

static void clearall()
{
    grab_keys(FALSE);
    destroytree(firstnode);
    firstnode = NULL;
    grab_keys(TRUE);
}

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

static PyObject *keyb_bind(Keyboard *self, PyObject *args)
{
    KeyBindingTree *tree = NULL, *t;
    gboolean conflict;
    PyObject *item, *tuple, *func;
    GList *keylist = NULL, *it;
    int i, s;

    CHECK_KEYBOARD(self, "grab");
    if (!PyArg_ParseTuple(args, "OO:grab", &tuple, &func))
	return NULL;

    if (!PyTuple_Check(tuple)) {
	PyErr_SetString(PyExc_ValueError, "expected a tuple of strings");
	goto binderror;
    }
    if (!PyCallable_Check(func)) {
	PyErr_SetString(PyExc_ValueError, "expected a callable object");
	goto binderror;
    }

    s = PyTuple_GET_SIZE(tuple);
    if (s <= 0) {
	PyErr_SetString(PyExc_ValueError, "expected a tuple of strings");
	goto binderror;
    }

    for (i = 0; i < s; ++i) {
	item = PyTuple_GET_ITEM(tuple, i);
	if (!PyString_Check(item)) {
	    PyErr_SetString(PyExc_ValueError, "expected a tuple of strings");
	    goto binderror;
	}
	keylist = g_list_append(keylist,
				g_strdup(PyString_AsString(item)));
    }

    if (!(tree = buildtree(keylist))) {
	PyErr_SetString(PyExc_ValueError, "invalid binding");
	goto binderror;
    }

    t = find(tree, &conflict);
    if (conflict) {
	PyErr_SetString(PyExc_ValueError, "conflict with binding");
	goto binderror;
    }
    if (t != NULL) {
	/* already bound to something */
	PyErr_SetString(PyExc_ValueError, "keychain is already bound");
	goto binderror;
    }

    /* grab the server here to make sure no key pressed go missed */
    XGrabServer(ob_display);
    XSync(ob_display, FALSE);

    grab_keys(FALSE);

    /* set the function */
    t = tree;
    while (t->first_child) t = t->first_child;
    t->func = func;
    Py_INCREF(func);

    /* assimilate this built tree into the main tree */
    assimilate(tree); // assimilation destroys/uses the tree

    grab_keys(TRUE); 

    XUngrabServer(ob_display);
    XFlush(ob_display);

    for (it = keylist; it != NULL; it = it->next)
	g_free(it->data);
    g_list_free(it);

    Py_INCREF(Py_None);
    return Py_None;

binderror:
    if (tree != NULL) destroytree(tree);
    for (it = keylist; it != NULL; it = it->next)
	g_free(it->data);
    g_list_free(it);
    return NULL;
}

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

