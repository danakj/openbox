#include "clientwrap.h"
#include "client.h"
#include "frame.h"
#include "engine.h"
#include "stacking.h"
#include "focus.h"
#include "prop.h"
#include <glib.h>

/***************************************************************************
 
   Define the type 'ClientWrap'

 ***************************************************************************/

#define IS_CWRAP(v)  ((v)->ob_type == &ClientWrapType)
#define IS_VALID_CWRAP(v) ((v)->client != NULL)
#define CHECK_CWRAP(self, funcname) { \
    if (!IS_CWRAP(self)) { \
        PyErr_SetString(PyExc_TypeError, \
			"descriptor '" funcname "' requires a 'Client' " \
			"object"); \
	return NULL; \
    } \
    if (!IS_VALID_CWRAP(self)) { \
        PyErr_SetString(PyExc_ValueError, \
			"This 'Client' is wrapping a client which no longer "\
			"exists."); \
        return NULL; \
    } \
}


staticforward PyTypeObject ClientWrapType;

/***************************************************************************
 
   Attribute methods
 
 ***************************************************************************/

static PyObject *cwrap_valid(ClientWrap *self, PyObject *args)
{
    if (!IS_CWRAP(self)) {
        PyErr_SetString(PyExc_TypeError,
			"descriptor 'valid' requires a 'Client' object");
	return NULL;
    }
    return PyInt_FromLong(self->client != NULL);
}

static PyObject *cwrap_title(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "title");
    if (!PyArg_ParseTuple(args, ":title"))
	return NULL;
    return PyString_FromString(self->client->title);
}

static PyObject *cwrap_setTitle(ClientWrap *self, PyObject *args)
{
    char *title;

    CHECK_CWRAP(self, "setTitle");
    if (!PyArg_ParseTuple(args, "s:setTitle", &title))
	return NULL;

    PROP_SETS(self->client->window, net_wm_name, utf8, title);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_iconTitle(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "iconTitle");
    if (!PyArg_ParseTuple(args, ":iconTitle"))
	return NULL;
    return PyString_FromString(self->client->icon_title);
}

static PyObject *cwrap_setIconTitle(ClientWrap *self, PyObject *args)
{
    char *title;

    CHECK_CWRAP(self, "setIconTitle");
    if (!PyArg_ParseTuple(args, "s:setIconTitle", &title))
	return NULL;

    PROP_SETS(self->client->window, net_wm_icon_name, utf8, title);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_desktop(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "desktop");
    if (!PyArg_ParseTuple(args, ":desktop"))
	return NULL;
    return PyInt_FromLong(self->client->desktop);
}

static PyObject *cwrap_setDesktop(ClientWrap *self, PyObject *args)
{
    int desktop;
    CHECK_CWRAP(self, "setDesktop");
    if (!PyArg_ParseTuple(args, "i:setDesktop", &desktop))
	return NULL;
    client_set_desktop(self->client, desktop);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_resName(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "resName");
    if (!PyArg_ParseTuple(args, ":resName"))
	return NULL;
    return PyString_FromString(self->client->res_name);
}

static PyObject *cwrap_resClass(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "resClass");
    if (!PyArg_ParseTuple(args, ":resClass"))
	return NULL;
    return PyString_FromString(self->client->res_class);
}

static PyObject *cwrap_role(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "role");
    if (!PyArg_ParseTuple(args, ":role"))
	return NULL;
    return PyString_FromString(self->client->role);
}

static PyObject *cwrap_transient(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "transient");
    if (!PyArg_ParseTuple(args, ":transient"))
	return NULL;
    return PyInt_FromLong(!!self->client->transient);
}

static PyObject *cwrap_transientFor(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "transientFor");
    if (!PyArg_ParseTuple(args, ":transientFor"))
	return NULL;
    if (self->client->transient_for != NULL)
	return clientwrap_new(self->client->transient_for);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_transients(ClientWrap *self, PyObject *args)
{
    PyObject *tuple;
    GSList *it;
    guint i, s;

    CHECK_CWRAP(self, "transients");
    if (!PyArg_ParseTuple(args, ":transients"))
	return NULL;
    s = g_slist_length(self->client->transients);
    tuple = PyTuple_New(s);
    for (i = 0, it = self->client->transients; i < s; ++i, it = it->next)
	PyTuple_SET_ITEM(tuple, i, clientwrap_new(it->data));
    return tuple;
}

static PyObject *cwrap_type(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "type");
    if (!PyArg_ParseTuple(args, ":type"))
	return NULL;
    return PyInt_FromLong(self->client->type);
}

static PyObject *cwrap_normal(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "normal");
    if (!PyArg_ParseTuple(args, ":normal"))
	return NULL;
    return PyInt_FromLong(!!client_normal(self->client));
}

static PyObject *cwrap_area(ClientWrap *self, PyObject *args)
{
    PyObject *tuple;

    CHECK_CWRAP(self, "area");
    if (!PyArg_ParseTuple(args, ":area"))
	return NULL;
    tuple = PyTuple_New(4);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(self->client->frame->area.x));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(self->client->frame->area.y));
    PyTuple_SET_ITEM(tuple, 2,
		     PyInt_FromLong(self->client->frame->area.width));
    PyTuple_SET_ITEM(tuple, 3,
		     PyInt_FromLong(self->client->frame->area.height));
    return tuple;
}

static PyObject *cwrap_setArea(ClientWrap *self, PyObject *args)
{
    int x, y, w, h, final = TRUE;

    CHECK_CWRAP(self, "setArea");
    if (!PyArg_ParseTuple(args, "(iiii)|i:setArea", &x, &y, &w, &h, &final))
	return NULL;

    frame_frame_gravity(self->client->frame, &x, &y);
    w -= self->client->frame->size.left + self->client->frame->size.right;
    h -= self->client->frame->size.top + self->client->frame->size.bottom;
    client_configure(self->client, Corner_TopLeft, x, y, w, h, TRUE, final);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_clientArea(ClientWrap *self, PyObject *args)
{
    PyObject *tuple;

    CHECK_CWRAP(self, "clientArea");
    if (!PyArg_ParseTuple(args, ":clientArea"))
	return NULL;
    tuple = PyTuple_New(4);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(self->client->area.x));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(self->client->area.y));
    PyTuple_SET_ITEM(tuple, 2, PyInt_FromLong(self->client->area.width));
    PyTuple_SET_ITEM(tuple, 3, PyInt_FromLong(self->client->area.height));
    return tuple;
}

static PyObject *cwrap_setClientArea(ClientWrap *self, PyObject *args)
{
    int x, y, w, h;

    CHECK_CWRAP(self, "setClientArea");
    if (!PyArg_ParseTuple(args, "(iiii)|i:setClientArea", &x, &y, &w, &h))
	return NULL;

    client_configure(self->client, Corner_TopLeft, x, y, w, h, TRUE, TRUE);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_frameSize(ClientWrap *self, PyObject *args)
{
    PyObject *tuple;

    CHECK_CWRAP(self, "frameSize");
    if (!PyArg_ParseTuple(args, ":frameSize"))
	return NULL;
    tuple = PyTuple_New(4);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(self->client->frame->size.left));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(self->client->frame->size.top));
    PyTuple_SET_ITEM(tuple, 2,
		     PyInt_FromLong(self->client->frame->size.right));
    PyTuple_SET_ITEM(tuple, 3,
		     PyInt_FromLong(self->client->frame->size.bottom));
    return tuple;
}

static PyObject *cwrap_strut(ClientWrap *self, PyObject *args)
{
    PyObject *tuple;

    CHECK_CWRAP(self, "strut");
    if (!PyArg_ParseTuple(args, ":strut"))
	return NULL;
    tuple = PyTuple_New(4);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(self->client->strut.left));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(self->client->strut.top));
    PyTuple_SET_ITEM(tuple, 2, PyInt_FromLong(self->client->strut.right));
    PyTuple_SET_ITEM(tuple, 3, PyInt_FromLong(self->client->strut.bottom));
    return tuple;
}

static PyObject *cwrap_logicalSize(ClientWrap *self, PyObject *args)
{
    PyObject *tuple;

    CHECK_CWRAP(self, "logicalSize");
    if (!PyArg_ParseTuple(args, ":logicalSize"))
	return NULL;
    tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0,
		     PyInt_FromLong(self->client->logical_size.width));
    PyTuple_SET_ITEM(tuple, 1,
		     PyInt_FromLong(self->client->logical_size.height));
    return tuple;
}

static PyObject *cwrap_canFocus(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "canFocus");
    if (!PyArg_ParseTuple(args, ":canFocus"))
	return NULL;
    return PyInt_FromLong(!!self->client->can_focus);
}

static PyObject *cwrap_focus(ClientWrap *self, PyObject *args)
{
    int focus = TRUE;

    CHECK_CWRAP(self, "focus");
    if (!PyArg_ParseTuple(args, "|i:focus", &focus))
	return NULL;
    if (focus)
	return PyInt_FromLong(!!client_focus(self->client));
    else {
	if (focus_client == self->client)
	    client_unfocus(self->client);
	Py_INCREF(Py_None);
	return Py_None;
    }
}

static PyObject *cwrap_focused(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "focused");
    if (!PyArg_ParseTuple(args, ":focused"))
	return NULL;
    return PyInt_FromLong(!!self->client->focused);
}

static PyObject *cwrap_visible(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "visible");
    if (!PyArg_ParseTuple(args, ":visible"))
	return NULL;
    return PyInt_FromLong(!!self->client->frame->visible);
}

static PyObject *cwrap_setVisible(ClientWrap *self, PyObject *args)
{
    int show;

    CHECK_CWRAP(self, "setVisible");
    if (!PyArg_ParseTuple(args, "i:setVisible", &show))
	return NULL;
    if (show)
	engine_frame_show(self->client->frame);
    else
	engine_frame_hide(self->client->frame);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_modal(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "modal");
    if (!PyArg_ParseTuple(args, ":modal"))
	return NULL;
    return PyInt_FromLong(!!self->client->modal);
}

static PyObject *cwrap_setModal(ClientWrap *self, PyObject *args)
{
    int modal;

    CHECK_CWRAP(self, "setModal");
    if (!PyArg_ParseTuple(args, "i:setModal", &modal))
	return NULL;

    client_set_state(self->client,
		     (modal ? prop_atoms.net_wm_state_add :
		      prop_atoms.net_wm_state_remove),
		     prop_atoms.net_wm_state_modal, 0);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_shaded(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "shaded");
    if (!PyArg_ParseTuple(args, ":shaded"))
	return NULL;
    return PyInt_FromLong(!!self->client->shaded);
}

static PyObject *cwrap_setShaded(ClientWrap *self, PyObject *args)
{
    int shaded;

    CHECK_CWRAP(self, "setShaded");
    if (!PyArg_ParseTuple(args, "i:setShaded", &shaded))
	return NULL;

    client_shade(self->client, shaded);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_iconic(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "iconic");
    if (!PyArg_ParseTuple(args, ":iconic"))
	return NULL;
    return PyInt_FromLong(!!self->client->iconic);
}

static PyObject *cwrap_setIconic(ClientWrap *self, PyObject *args)
{
    int iconify, current = TRUE;

    CHECK_CWRAP(self, "setIconic");
    if (!PyArg_ParseTuple(args, "i|i:setIconic", &iconify, &current))
	return NULL;

    client_iconify(self->client, iconify, current);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_maximizedHorz(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "maximizedHorz");
    if (!PyArg_ParseTuple(args, ":maximizedHorz"))
	return NULL;
    return PyInt_FromLong(!!self->client->max_horz);
}

static PyObject *cwrap_setMaximizedHorz(ClientWrap *self, PyObject *args)
{
    int max;

    CHECK_CWRAP(self, "setMaximizedHorz");
    if (!PyArg_ParseTuple(args, "i:setMaximizedHorz", &max))
	return NULL;

    client_set_state(self->client,
		     (max ? prop_atoms.net_wm_state_add :
		      prop_atoms.net_wm_state_remove),
		     prop_atoms.net_wm_state_maximized_horz, 0);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_maximizedVert(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "maximizedVert");
    if (!PyArg_ParseTuple(args, ":maximizedVert"))
	return NULL;
    return PyInt_FromLong(!!self->client->max_vert);
}

static PyObject *cwrap_setMaximizedVert(ClientWrap *self, PyObject *args)
{
    int max;

    CHECK_CWRAP(self, "setMaximizedVert");
    if (!PyArg_ParseTuple(args, "i:setMaximizedVert", &max))
	return NULL;

    client_set_state(self->client,
		     (max ? prop_atoms.net_wm_state_add :
		      prop_atoms.net_wm_state_remove),
		     prop_atoms.net_wm_state_maximized_vert, 0);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_maximized(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "maximized");
    if (!PyArg_ParseTuple(args, ":maximized"))
	return NULL;
    return PyInt_FromLong(self->client->max_vert || self->client->max_horz);
}

static PyObject *cwrap_setMaximized(ClientWrap *self, PyObject *args)
{
    int max;

    CHECK_CWRAP(self, "setMaximized");
    if (!PyArg_ParseTuple(args, "i:setMaximized", &max))
	return NULL;

    client_set_state(self->client,
		     (max ? prop_atoms.net_wm_state_add :
		      prop_atoms.net_wm_state_remove),
		     prop_atoms.net_wm_state_maximized_vert,
		     prop_atoms.net_wm_state_maximized_horz);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_fullscreen(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "fullscreen");
    if (!PyArg_ParseTuple(args, ":fullscreen"))
	return NULL;
    return PyInt_FromLong(!!self->client->fullscreen);
}

static PyObject *cwrap_setFullscreen(ClientWrap *self, PyObject *args)
{
    int fs;

    CHECK_CWRAP(self, "setFullscreen");
    if (!PyArg_ParseTuple(args, "i:setFullscreen", &fs))
	return NULL;

    client_set_state(self->client,
		     (fs ? prop_atoms.net_wm_state_add :
		      prop_atoms.net_wm_state_remove),
		     prop_atoms.net_wm_state_fullscreen, 0);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_stacking(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "stacking");
    if (!PyArg_ParseTuple(args, ":stacking"))
	return NULL;
    return PyInt_FromLong(self->client->above ? 1 :
			  self->client->below ? -1 : 0);
}

static PyObject *cwrap_setStacking(ClientWrap *self, PyObject *args)
{
    int stack;

    CHECK_CWRAP(self, "setStacking");
    if (!PyArg_ParseTuple(args, "i:setStacking", &stack))
	return NULL;
    client_set_state(self->client,
		     (stack > 0 ? prop_atoms.net_wm_state_add :
		      prop_atoms.net_wm_state_remove),
		     prop_atoms.net_wm_state_above, 0);
    client_set_state(self->client,
		     (stack < 0 ? prop_atoms.net_wm_state_add :
		      prop_atoms.net_wm_state_remove),
		     prop_atoms.net_wm_state_below, 0);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_raiseWindow(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "raiseWindow");
    if (!PyArg_ParseTuple(args, ":raiseWindow"))
	return NULL;
    stacking_raise(self->client);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_lowerWindow(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "lowerWindow");
    if (!PyArg_ParseTuple(args, ":lowerWindow"))
	return NULL;
    stacking_lower(self->client);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_skipPager(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "skipPager");
    if (!PyArg_ParseTuple(args, ":skipPager"))
	return NULL;
    return PyInt_FromLong(!!self->client->skip_pager);
}

static PyObject *cwrap_setSkipPager(ClientWrap *self, PyObject *args)
{
    int skip;

    CHECK_CWRAP(self, "setSkipPager");
    if (!PyArg_ParseTuple(args, "i:setSkipPager", &skip))
	return NULL;

    client_set_state(self->client,
		     (skip ? prop_atoms.net_wm_state_add :
		      prop_atoms.net_wm_state_remove),
		     prop_atoms.net_wm_state_skip_pager, 0);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_skipTaskbar(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "skipTaskbar");
    if (!PyArg_ParseTuple(args, ":skipTaskbar"))
	return NULL;
    return PyInt_FromLong(!!self->client->skip_taskbar);
}

static PyObject *cwrap_setSkipTaskbar(ClientWrap *self, PyObject *args)
{
    int skip;

    CHECK_CWRAP(self, "setSkipTaskbar");
    if (!PyArg_ParseTuple(args, "i:setSkipTaskbar", &skip))
	return NULL;

    client_set_state(self->client,
		     (skip ? prop_atoms.net_wm_state_add :
		      prop_atoms.net_wm_state_remove),
		     prop_atoms.net_wm_state_skip_taskbar, 0);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_disableDecorations(ClientWrap *self, PyObject *args)
{
    int title, handle, border;

    CHECK_CWRAP(self, "disableDecorations");
    if (!PyArg_ParseTuple(args, "iii:disableDecorations", &title, &handle,
			  &border))
	return NULL;

    self->client->disabled_decorations = 0;
    if (title) self->client->disabled_decorations |= Decor_Titlebar;
    if (handle) self->client->disabled_decorations |= Decor_Handle;
    if (border) self->client->disabled_decorations |= Decor_Border;
    client_setup_decor_and_functions(self->client);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_close(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "close");
    if (!PyArg_ParseTuple(args, ":close"))
	return NULL;
    client_close(self->client);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_window(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "window");
    if (!PyArg_ParseTuple(args, ":window"))
	return NULL;
    return PyInt_FromLong(self->client->window);
}

#define METH(n, d) {#n, (PyCFunction)cwrap_##n, METH_VARARGS, #d}

static PyMethodDef ClientWrapMethods[] = {
    METH(valid,
	 "Returns if the Client instance is still valid. Client instances are "
	 "marked as invalid when the Client they are associated is "
	 "closed/destroyed/released."),
    METH(title,
	 "Returns the client's title."),
    METH(setTitle,
	 "Change the client's title to the given string. This change will be "
	 "overwritten if/when the client changes its title."),
    METH(iconTitle,
	 "Returns's the client's icon title. The icon title is the title to "
	 "be displayed when the client is iconified."),
    METH(setIconTitle,
	 "Change the client's icon title to the given string. This change "
	 "will be overwritten if/when the client changes its icon title."),
    METH(desktop,
	 "Returns the desktop on which the client is visible. This value will "
	 "always be in the range [0, ob.Openbox.numDesktops()), unless it is "
	 "0xffffffff. A value of 0xffffffff indicates the client is visible "
	 "on all desktops."),
    METH(setDesktop,
	 "Moves the client to the specified desktop. The desktop must be in "
	 "the range [0, ob.Openbox.numDesktops()), unless it is 0xffffffff. A "
	 "value of 0xffffffff indicates the client is visible on all "
	 "desktops."),
    METH(resName,
	 "Returns the resouce name of the client. The resource name is meant "
	 "to provide an instance name for the client."),
    METH(resClass,
	 "Returns the resouce class of the client. The resource class is "
	 "meant to provide the genereal class of the application. e.g. "
	 "'Emacs', 'Xterm', 'XClock', 'XLoad', and so on."),
    METH(role,
	 "Returns the client's role. The role is meant to distinguish between "
	 "different windows of an application. Each window should have a "
	 "unique role."),
    METH(transient,
	 "Returns True or False describing if the client is a transient "
	 "window. Transient windows are 'temporary' windows, such as "
	 "preference dialogs, and usually have a parent window, which can be "
	 "found from transientFor()."),
    METH(transientFor,
	 "Returns the client for which this client is a transient. See "
	 "transient() for a description of transience."),
    METH(transients,
	 "Returns a tuple containing all the Clients which are transients of "
	 "this window. See transient() for a description of transience."),
    METH(type,
	 "Returns the logical type of the window. This is one of the "
	 "ClientType constants. See also normal()."),
    METH(normal,
	 "Returns True or False for if the client is a 'normal' window. "
	 "Normal windows make up most applications. Non-normal windows have "
	 "special rules applied to them at times such as for focus handling. "
	 "An example of a non-normal window is 'gnome-panel'. This value is "
	 "determined from the client's type(), but does not imply that the "
	 "window is ClientType.Normal. Rather this is a more generic "
	 "definition of 'normal' windows, and includes dialogs and others."),
    METH(area,
	 "Returns the area of the screen which the client occupies. It may be "
	 "important to note that this is the position and size of the client "
	 "*with* its decorations. If you want the underlying position and "
	 "size of the client itself, you should use clientArea(). See also "
	 "logicalSize()."),
    
    METH(setArea,
	 "Sets the client's area, moving and resizing it as specified (or as "
	 "close as can be accomidated)."),
    METH(clientArea,
	 "Returns the area of the screen which the client considers itself to "
	 "be occupying. This value is not what you see and should not be used "
	 "for most things (it should, for example, be used for persisting a "
	 "client's dimentions across sessions). See also area()."),
    METH(setClientArea,
	 "Sets the area of the screen which the client considers itself to be "
	 "occupying. This is not the on-screen visible position and size, and "
	 "should be used with care. You probably want to use setArea() to "
	 "adjust the client. This should be used if you want the client "
	 "window (inside the decorations) to be a specific size. Adjusting "
	 "the client's position with this function is probably always a bad "
	 "idea, because of window gravity."),
    METH(strut,
	 "Returns the application's specified strut. The strut is the amount "
	 "of space that should be reserved for the application on each side "
	 "of the screen."),
    METH(frameSize,
	 "Returns the size of the decorations around the client window."),
    METH(logicalSize,
	 "Returns the client's logical size. The logical size is the client's "
	 "size in more user friendly terms. For many apps this is simply the "
	 "size of the client in pixels, however for some apps this will "
	 "differ (e.g. terminal emulators). This value should be used when "
	 "displaying an applications size to the user."),
    METH(canFocus,
	 "Returns True or False for if the client can be focused."),
    METH(focus,
	 "Focuses (or unfocuses) the client window. Windows which return "
	 "False for canFocus() or visible() cannot be focused. When this "
	 "function returns, the client's focused() state will not be changed "
	 "yet. This only sends the request through the X server. You should "
	 "wait for the hooks.focused hook to fire, and not assume the client "
	 "has been focused."),
    METH(focused,
	 "Returns True or False for if the client has the input focus."),
    METH(visible,
	 "Returns True or False for if the client is visible. A client is not "
	 "visible if it is iconic() or if its desktop() is not visible."),
    METH(setVisible,
	 "Shows or hides the client. This has no effect if its current "
	 "visible() state is requested."),
    METH(modal,
	 "Returns True or False for if the client is a modal window. Modal "
	 "windows indicate that they must be dealt with before the program "
	 "can continue. When a modal window is a transient(), its "
	 "transientFor() client cannot be focused or raised above it."),
    METH(setModal,
	 "Make the client window modal or non-modal."),
    METH(shaded,
	 "Returns True or False for if the client is shaded. Shaded windows "
	 "have only their titlebar decorations showing."),
    METH(setShaded,
	 "Shade or unshade the client. Shaded windows have only their "
	 "titlebar decorations showing. Windows which do not have a titlebar "
	 "cannot be shaded."),
    METH(iconic,
	 "Returns True or False for if the window is iconified. Iconified "
	 "windows are not visible on any desktops."),
    METH(setIconic,
	 "Iconifies or restores the client window. Iconified windows are not "
	 "visible on any desktops. Iconified windows can be restored to the "
	 "currently visible desktop or to their original (native) desktop."),
    METH(maximizedHorz,
	 "Returns whether the client is maximized in the horizontal "
	 "direction."),
    METH(setMaximizedHorz,
	 "Maximizes or restores a client horizontally."),
    METH(maximizedVert,
	 "Returns whether the client is maximized in the vertical direction."),
    METH(setMaximizedVert,
	 "Maximizes or restores a client vertically."),
    METH(maximized,
	 "Returns whether the client is maximized in the horizontal or "
	 "vertical direction."),
    METH(setMaximized,
	 "Maximizes or restores a client vertically and horzontally."),
    METH(fullscreen,
	 "Returns if the client is in fullscreen mode. Fullscreen windows are "
	 "kept above all other windows and are stretched to fill the entire "
	 "physical display."),
    METH(setFullscreen,
	 "Set a client into or out of fullscreen mode. Fullscreen windows are "
	 "kept above all other windows and are stretched to fill the entire "
	 "physical display."),
    METH(stacking,
	 "Returns if the client will be stacked above/below other clients in "
	 "the same layer."),
    METH(setStacking,
	 "Set how the client will be stacked according to other clients in "
	 "its layer."),
    METH(raiseWindow,
	 "Raises the window to the top of its stacking layer."),
    METH(lowerWindow,
	 "Lowers the window to the bottom of its stacking layer."),
    METH(skipPager,
	 "Returns if the client has requested to be skipped (not displayed) "
	 "by pagers."),
    METH(setSkipPager,
	 "Set whether the client should be skipped (not displayed) by "
	 "pagers."),
    METH(skipTaskbar,
	 "Returns if the client has requested to be skipped (not displayed) "
	 "by taskbars."),
    METH(setSkipTaskbar,
	 "Set whether the client should be skipped (not displayed) by "
	 "taskbars."),
    METH(disableDecorations,
	 "Choose which decorations to disable on the client. Note that "
	 "decorations can only be disabled, and decorations that would "
	 "normally not be shown cannot be added. These values may have "
	 "slightly different meanings in different theme engines."),
    METH(close,
	 "Requests the client to close its window."),
    METH(window,
	 "Returns the client's window id. This is the id by which the X "
	 "server knows the client."),
    { NULL, NULL, 0, NULL }
};

/***************************************************************************
 
   Type methods/struct
 
 ***************************************************************************/

/*static PyObject *cwrap_getattr(ClientWrap *self, char *name)
{
    CHECK_CWRAP(self, "getattr");
    return Py_FindMethod(ClientWrapAttributeMethods, (PyObject*)self, name);
}*/

static void cwrap_dealloc(ClientWrap *self)
{
    if (self->client != NULL)
	self->client->wrap = NULL;
    PyObject_Del((PyObject*) self);
}

static PyObject *cwrap_repr(ClientWrap *self)
{
     CHECK_CWRAP(self, "repr");
     return PyString_FromFormat("Client: 0x%x", (guint)self->client->window);
}

static int cwrap_compare(ClientWrap *self, ClientWrap *other)
{
    Window w1, w2;
    if (!IS_VALID_CWRAP(self)) {
	PyErr_SetString(PyExc_ValueError,
			"This 'Client' is wrapping a client which no longer "
			"exists.");
    }

    w1 = self->client->window;
    w2 = other->client->window;
    return w1 > w2 ? 1 : w1 < w2 ? -1 : 0;
}

static PyTypeObject ClientWrapType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Client",
    sizeof(ClientWrap),
    0,
    (destructor) cwrap_dealloc,     /*tp_dealloc*/
    0,                              /*tp_print*/
    0,                              /*tp_getattr*/
    0,                              /*tp_setattr*/
    (cmpfunc) cwrap_compare,        /*tp_compare*/
    (reprfunc) cwrap_repr,          /*tp_repr*/
    0,                              /*tp_as_number*/
    0,                              /*tp_as_sequence*/
    0,                              /*tp_as_mapping*/
    0,                              /*tp_hash */
};


/***************************************************************************
 
   Define the type 'ClientType'

 ***************************************************************************/

#define IS_CTYPE(v)  ((v)->ob_type == &ClientTypeType)
#define CHECK_CTYPE(self, funcname) { \
    if (!IS_CTYPE(self)) { \
        PyErr_SetString(PyExc_TypeError, \
			"descriptor '" funcname "' requires a 'ClientType' " \
			"object"); \
	return NULL; \
    } \
}

staticforward PyTypeObject ClientTypeType;

typedef struct ClientType {
    PyObject_HEAD
} ClientType;

static void ctype_dealloc(PyObject *self)
{
    PyObject_Del(self);
}

static PyObject *ctype_getattr(ClientType *self, char *name)
{
    struct S { char *name; int val; };
    struct S s[] = {
	{ "Normal", Type_Normal },
	{ "Dialog", Type_Dialog },
	{ "Desktop", Type_Desktop },
	{ "Dock", Type_Dock },
	{ "Toolbar", Type_Toolbar },
	{ "Menu", Type_Menu },
	{ "Utility", Type_Utility },
	{ "Splash", Type_Splash },
	{ NULL, 0 } };
    int i;

    CHECK_CTYPE(self, "__getattr__");

    for (i = 0; s[i].name != NULL; ++i)
	if (!strcmp(s[i].name, name))
	    return PyInt_FromLong(s[i].val);
    PyErr_SetString(PyExc_AttributeError, "invalid attribute");
    return NULL;
}

static PyTypeObject ClientTypeType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Type",
    sizeof(ClientType),
    0,
    (destructor) ctype_dealloc,     /*tp_dealloc*/
    0,                              /*tp_print*/
    (getattrfunc) ctype_getattr,    /*tp_getattr*/
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

void clientwrap_startup()
{
    PyObject *ob, *obdict, *type;

    ClientWrapType.ob_type = &PyType_Type;
    ClientWrapType.tp_methods = ClientWrapMethods;
    PyType_Ready(&ClientWrapType);

    ClientTypeType.ob_type = &PyType_Type;
    PyType_Ready(&ClientTypeType);

    /* get the ob module/dict */
    ob = PyImport_ImportModule("ob"); /* new */
    g_assert(ob != NULL);
    obdict = PyModule_GetDict(ob); /* borrowed */
    g_assert(obdict != NULL);

    /* add the Client type to the ob module */
    PyDict_SetItemString(obdict, "Client", (PyObject*) &ClientWrapType);

    /* add an instance of ClientType */
    type = (PyObject*) PyObject_New(ClientType, &ClientTypeType);
    PyDict_SetItemString(obdict, "ClientType", type);
    Py_DECREF(type);

    Py_DECREF(ob);
}

void clientwrap_shutdown()
{
}

PyObject *clientwrap_new(Client *client)
{
    g_assert(client != NULL);

    if (client->wrap != NULL) {
	/* already has a wrapper! */
	Py_INCREF((PyObject*) client->wrap);
    } else {
	client->wrap = PyObject_New(ClientWrap, &ClientWrapType);
	client->wrap->client = client;
    }
    return (PyObject*) client->wrap;
}
