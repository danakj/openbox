#include "clientwrap.h"
#include "client.h"
#include "frame.h"
#include "stacking.h"
#include "focus.h"
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

static PyObject *cwrap_window(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "window");
    if (!PyArg_ParseTuple(args, ":window"))
	return NULL;
    return PyInt_FromLong(self->client->window);
}

static PyObject *cwrap_group(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "group");
    if (!PyArg_ParseTuple(args, ":group"))
	return NULL;
    return PyInt_FromLong(self->client->group);
}

static PyObject *cwrap_parent(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "parent");
    if (!PyArg_ParseTuple(args, ":parent"))
	return NULL;
    if (self->client->transient_for != NULL)
	return clientwrap_new(self->client->transient_for);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_children(ClientWrap *self, PyObject *args)
{
    PyObject *list;
    GSList *it;
    guint i, s;

    CHECK_CWRAP(self, "children");
    if (!PyArg_ParseTuple(args, ":children"))
	return NULL;
    s = g_slist_length(self->client->transients);
    list = PyList_New(s);
    for (i = 0, it = self->client->transients; i < s; ++i, it = it->next)
	PyList_SET_ITEM(list, i, clientwrap_new(it->data));
    return list;
}

static PyObject *cwrap_desktop(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "desktop");
    if (!PyArg_ParseTuple(args, ":desktop"))
	return NULL;
    return PyInt_FromLong(self->client->desktop);
}

static PyObject *cwrap_title(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "title");
    if (!PyArg_ParseTuple(args, ":title"))
	return NULL;
    return PyString_FromString(self->client->title);
}

static PyObject *cwrap_iconTitle(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "iconTitle");
    if (!PyArg_ParseTuple(args, ":iconTitle"))
	return NULL;
    return PyString_FromString(self->client->icon_title);
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

static PyObject *cwrap_type(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "type");
    if (!PyArg_ParseTuple(args, ":type"))
	return NULL;
    return PyInt_FromLong(self->client->type);
}

static PyObject *cwrap_area(ClientWrap *self, PyObject *args)
{
    PyObject *tuple;

    CHECK_CWRAP(self, "area");
    if (!PyArg_ParseTuple(args, ":area"))
	return NULL;
    tuple = PyTuple_New(4);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(self->client->area.x));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(self->client->area.y));
    PyTuple_SET_ITEM(tuple, 2, PyInt_FromLong(self->client->area.width));
    PyTuple_SET_ITEM(tuple, 3, PyInt_FromLong(self->client->area.height));
    return tuple;
}

static PyObject *cwrap_screenArea(ClientWrap *self, PyObject *args)
{
    PyObject *tuple;

    CHECK_CWRAP(self, "screenArea");
    if (!PyArg_ParseTuple(args, ":screenArea"))
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

static PyObject *cwrap_minRatio(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "minRatio");
    if (!PyArg_ParseTuple(args, ":minRatio"))
	return NULL;
    return PyFloat_FromDouble(self->client->min_ratio);
}

static PyObject *cwrap_maxRatio(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "maxRatio");
    if (!PyArg_ParseTuple(args, ":maxRatio"))
	return NULL;
    return PyFloat_FromDouble(self->client->max_ratio);
}

static PyObject *cwrap_minSize(ClientWrap *self, PyObject *args)
{
    PyObject *tuple;

    CHECK_CWRAP(self, "minSize");
    if (!PyArg_ParseTuple(args, ":minSize"))
	return NULL;
    tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(self->client->min_size.width));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(self->client->min_size.height));
    return tuple;
}

static PyObject *cwrap_maxSize(ClientWrap *self, PyObject *args)
{
    PyObject *tuple;

    CHECK_CWRAP(self, "maxSize");
    if (!PyArg_ParseTuple(args, ":maxSize"))
	return NULL;
    tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(self->client->max_size.width));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(self->client->max_size.height));
    return tuple;
}

static PyObject *cwrap_sizeIncrement(ClientWrap *self, PyObject *args)
{
    PyObject *tuple;

    CHECK_CWRAP(self, "sizeIncrement");
    if (!PyArg_ParseTuple(args, ":sizeIncrement"))
	return NULL;
    tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(self->client->size_inc.width));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(self->client->size_inc.height));
    return tuple;
}

static PyObject *cwrap_baseSize(ClientWrap *self, PyObject *args)
{
    PyObject *tuple;

    CHECK_CWRAP(self, "baseSize");
    if (!PyArg_ParseTuple(args, ":baseSize"))
	return NULL;
    tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(self->client->base_size.width));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(self->client->base_size.height));
    return tuple;
}

static PyObject *cwrap_gravity(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "gravity");
    if (!PyArg_ParseTuple(args, ":gravity"))
	return NULL;
    return PyInt_FromLong(self->client->gravity);
}

static PyObject *cwrap_canClose(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "canClose");
    if (!PyArg_ParseTuple(args, ":canClose"))
	return NULL;
    return PyInt_FromLong(self->client->delete_window ? 1 : 0);
}

static PyObject *cwrap_positionRequested(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "positionRequested");
    if (!PyArg_ParseTuple(args, ":positionRequested"))
	return NULL;
    return PyInt_FromLong(self->client->positioned ? 1 : 0);
}

static PyObject *cwrap_canFocus(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "canFocus");
    if (!PyArg_ParseTuple(args, ":canFocus"))
	return NULL;
    return PyInt_FromLong(self->client->can_focus ||
			  self->client->focus_notify);
}

static PyObject *cwrap_urgent(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "urgent");
    if (!PyArg_ParseTuple(args, ":urgent"))
	return NULL;
    return PyInt_FromLong(self->client->urgent ? 1 : 0);
}

static PyObject *cwrap_focused(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "focused");
    if (!PyArg_ParseTuple(args, ":focused"))
	return NULL;
    return PyInt_FromLong(self->client->focused ? 1 : 0);
}

static PyObject *cwrap_modal(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "modal");
    if (!PyArg_ParseTuple(args, ":modal"))
	return NULL;
    return PyInt_FromLong(self->client->modal ? 1 : 0);
}

static PyObject *cwrap_shaded(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "shaded");
    if (!PyArg_ParseTuple(args, ":shaded"))
	return NULL;
    return PyInt_FromLong(self->client->shaded ? 1 : 0);
}

static PyObject *cwrap_iconic(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "iconic");
    if (!PyArg_ParseTuple(args, ":iconc"))
	return NULL;
    return PyInt_FromLong(self->client->iconic ? 1 : 0);
}

static PyObject *cwrap_maximizedVertical(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "maximizedVertical");
    if (!PyArg_ParseTuple(args, ":maximizedVertical"))
	return NULL;
    return PyInt_FromLong(self->client->max_vert ? 1 : 0);
}

static PyObject *cwrap_maximizedHorizontal(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "maximizedHorizontal");
    if (!PyArg_ParseTuple(args, ":maximizedHorizontal"))
	return NULL;
    return PyInt_FromLong(self->client->max_horz ? 1 : 0);
}

static PyObject *cwrap_skipPager(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "skipPager");
    if (!PyArg_ParseTuple(args, ":skipPager"))
	return NULL;
    return PyInt_FromLong(self->client->skip_pager ? 1 : 0);
}

static PyObject *cwrap_skipTaskbar(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "skipTaskbar");
    if (!PyArg_ParseTuple(args, ":skipTaskbar"))
	return NULL;
    return PyInt_FromLong(self->client->skip_taskbar ? 1 : 0);
}

static PyObject *cwrap_fullscreen(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "fullscreen");
    if (!PyArg_ParseTuple(args, ":fullscreen"))
	return NULL;
    return PyInt_FromLong(self->client->fullscreen ? 1 : 0);
}

static PyObject *cwrap_above(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "above");
    if (!PyArg_ParseTuple(args, ":above"))
	return NULL;
    return PyInt_FromLong(self->client->above ? 1 : 0);
}

static PyObject *cwrap_below(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "below");
    if (!PyArg_ParseTuple(args, ":below"))
	return NULL;
    return PyInt_FromLong(self->client->below ? 1 : 0);
}

static PyObject *cwrap_layer(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "layer");
    if (!PyArg_ParseTuple(args, ":layer"))
	return NULL;
    return PyInt_FromLong(self->client->layer);
}

static PyObject *cwrap_decorations(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "decorations");
    if (!PyArg_ParseTuple(args, ":decorations"))
	return NULL;
    return PyInt_FromLong(self->client->decorations);
}

static PyObject *cwrap_disabledDecorations(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "disabledDecorations");
    if (!PyArg_ParseTuple(args, ":disabledDecorations"))
	return NULL;
    return PyInt_FromLong(self->client->disabled_decorations);
}

static PyObject *cwrap_functions(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "functions");
    if (!PyArg_ParseTuple(args, ":functions"))
	return NULL;
    return PyInt_FromLong(self->client->functions);
}

static PyObject *cwrap_visible(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "visible");
    if (!PyArg_ParseTuple(args, ":visible"))
	return NULL;
    return PyInt_FromLong(self->client->frame->visible ? 1 : 0);
}

static PyObject *cwrap_decorationSize(ClientWrap *self, PyObject *args)
{
    PyObject *tuple;

    CHECK_CWRAP(self, "decorationSize");
    if (!PyArg_ParseTuple(args, ":decorationSize"))
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

static PyObject *cwrap_normal(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "normal");
    if (!PyArg_ParseTuple(args, ":normal"))
	return NULL;
    return PyInt_FromLong(client_normal(self->client) ? 1 : 0);
}

static PyObject *cwrap_setVisible(ClientWrap *self, PyObject *args)
{
    int i;

    CHECK_CWRAP(self, "setVisible");
    if (!PyArg_ParseTuple(args, "i:setVisible", &i))
	return NULL;
    if (i)
	frame_show(self->client->frame);
    else
	frame_hide(self->client->frame);
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

static PyObject *cwrap_focus(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "focus");
    if (!PyArg_ParseTuple(args, ":focus"))
	return NULL;
    return PyInt_FromLong(client_focus(self->client) ? 1 : 0);
}

static PyObject *cwrap_unfocus(ClientWrap *self, PyObject *args)
{
    CHECK_CWRAP(self, "unfocus");
    if (!PyArg_ParseTuple(args, ":unfocus"))
	return NULL;
    client_unfocus(self->client);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *cwrap_move(ClientWrap *self, PyObject *args)
{
    int x, y;
    int final = TRUE;
    CHECK_CWRAP(self, "move");
    if (!PyArg_ParseTuple(args, "ii|i:unfocus", &x, &y, &final))
	return NULL;
    /* get the client's position based on x,y for the frame */
    frame_frame_gravity(self->client->frame, &x, &y); 

    client_configure(self->client, Corner_TopLeft, x, y,
		     self->client->area.width, self->client->area.height,
		     TRUE, final);
    Py_INCREF(Py_None);
    return Py_None;
}

#define ATTRMETH(n, d) {#n, (PyCFunction)cwrap_##n, METH_VARARGS, #d}

static PyMethodDef ClientWrapMethods[] = {
    ATTRMETH(window,
	     "c.window() -- Returns the window id for the Client."),
    ATTRMETH(group,
	     "c.group() -- Returns the group id for the Client."),
    ATTRMETH(parent,
	     "c.parent() -- Returns the parent Client for the Client, or the "
	     "Client for which this Client is a transient. Returns None if it "
	     "is not a transient."),
    ATTRMETH(children,
	     "c.parent() -- Returns a list of child Clients for the Client, "
	     "or the Clients transients."),
    ATTRMETH(desktop,
	     "c.desktop() -- Returns the desktop on which the Client resides, "
	     "or 0xffffffff for 'all desktops'."),
    ATTRMETH(title,
	     "c.title() -- Returns the Client's title string. This is in "
	     "UTF-8 encoding."),
    ATTRMETH(iconTitle,
	     "c.iconTitle() -- Returns the Client's icon title string. This "
	     "is in UTF-8 encoding."),
    ATTRMETH(resName,
	     "c.resName() -- Returns the application's specified resource "
	     "name."),
    ATTRMETH(resClass,
	     "c.resClass() -- Returns the application's specified resource "
	     "class."),
    ATTRMETH(role,
	     "c.role() -- Returns the window's role, which should be unique "
	     "for every window of an application, if it is not empty."),
    ATTRMETH(type,
	     "c.type() -- Returns the window's type, one of the ob.Type_ "
	     "constants. This is the logical type of the window."),
    ATTRMETH(area,
	     "c.area() -- Returns the area rectangle for the Client in a "
	     "tuple. The tuple's format is (x, y, width, height). This is "
	     "not the area on-screen that the Client and frame occupies, but "
	     "rather, the position and size that the Client has requested, "
	     "before its gravity() and decorations have been applied. You "
	     "should use the c.screenArea() to get the actual on-screen area "
	     "for the Client."),
    ATTRMETH(screenArea,
	     "c.screenArea() -- Returns the on-screen area rectangle for the "
	     "Client in a tuple. The tuple's format is (x, y, width, height). "
	     "This is the actual position and size of the Client plus its "
	     "decorations."),
    ATTRMETH(strut,
	     "c.strut() -- Returns the strut requested by the Client in a "
	     "tuple. The format of the tuple is (left, top, right, bottom)."),
    ATTRMETH(logicalSize,
	     "c.logicalSize() -- Returns the logical size of the Client. This "
	     "is the 'user friendly' size for the Client, such as for an "
	     "xterm, it will be the number of characters in the xterm "
	     "instead of the number of pixels it takes up."),
    ATTRMETH(minRatio,
	     "c.minRatio() -- Returns the minimum width:height ratio for "
	     "the Client. A value of 0 implies no ratio enforcement."),
    ATTRMETH(maxRatio,
	     "c.maxRatio() -- Returns the maximum width:height ratio for "
	     "the Client. A value of 0 implies no ratio enforcement."),
    ATTRMETH(minSize,
	     "c.minSize() -- Returns the minimum size of the Client."),
    ATTRMETH(maxSize,
	     "c.maxSize() -- Returns the maximum size of the Client."),
    ATTRMETH(sizeIncrement,
	     "c.sizeIncrement() -- Returns the size increments in which the "
	     "Client must be resized."),
    ATTRMETH(baseSize,
	     "c.baseSize() -- Returns the base size of the Client, which is "
	     "subtracted from the Client's size before comparing to its "
	     "various sizing constraints."),
    ATTRMETH(gravity,
	     "c.gravity() -- Returns the gravity for the Client. One of the "
	     "ob.Gravity_ constants."),
    ATTRMETH(canClose,
	     "c.canClose() -- Returns whether or not the Client provides a "
	     "means for Openbox to request that it close."),
    ATTRMETH(positionRequested,
	     "c.positionRequested() -- Returns whether or not the Client has "
	     "requested a specified position on screen. When it has it should "
	     "probably not be placed using an algorithm when it is managed."),
    ATTRMETH(canFocus,
	     "c.canFocus() -- Returns whether or not the Client can be "
	     "given input focus."),
    ATTRMETH(urgent,
	     "c.urgent() -- Returns the urgent state of the window (on/off)."),
    ATTRMETH(focused,
	     "c.focused() -- Returns whether or not the Client has the input "
	     "focus."),
    ATTRMETH(modal,
	     "c.modal() -- Returns whether or not the Client is modal. A "
	     "modal Client implies that it needs to be closed before its "
	     "parent() can be used (focsed) again."),
    ATTRMETH(shaded,
	     "c.shaded() -- Returns whether or not the Client is shaded. "
	     "Shaded clients are hidden except for their titlebar."),
    ATTRMETH(iconic,
	     "c.iconic() -- Returns whether or not the Client is iconic. "
	     "Iconic windows are represented only by icons, or possibly "
	     "hidden entirely."),
    ATTRMETH(maximizedVertical,
	     "c.maximizedVertical() -- Returns whether or not the Client is "
	     "maxized vertically. When a Client is maximized it will expand "
	     "to fill as much of the screen as it can in that direction."),
    ATTRMETH(maximizedHorizontal,
	     "c.maximizedHorizontal() -- Returns whether or not the Client is "
	     "maxized horizontally. When a Client is maximized it will expand "
	     "to fill as much of the screen as it can in that direction."),
    ATTRMETH(skipPager,
	     "c.skipPager() -- Returns whether the Client as requested to be "
	     "skipped by pagers."),
    ATTRMETH(skipTaskbar,
	     "c.skipTaskbar() -- Returns whether the Client as requested to "
	     "be skipped by taskbars."),
    ATTRMETH(fullscreen,
	     "c.fullscreen() -- Returns whether the Client is in fullscreen "
	     "mode."),
    ATTRMETH(above,
	     "c.above() -- Returns whether the Client should be stacked above "
	     "other windows of the same type."),
    ATTRMETH(below,
	     "c.below() -- Returns whether the Client should be stacked below "
	     "other windows of the same type."),
    ATTRMETH(layer,
	     "c.layer() -- Returns the layer in which the window should be "
	     "stacked. This is one of the ob.Layer_ constants. Windows in "
	     "layers with higher values should be kept above windows in lower "
	     "valued layers."),
    ATTRMETH(decorations,
	     "c.decorations() -- Returns a mask of decorations which the "
	     "Client will be given. It is made up of the ob.Decor_ constants. "
	     "These can be turned off with the "
	     " disabledDecorations()."),
    ATTRMETH(disabledDecorations,
	     "c.disabledDecorations() -- returns a mask of decorations which "
	     "are disabled on the Client. This is made up of the ob.Decor_ "
	     "constants."),
    ATTRMETH(functions,
	     "ob.functions() -- Returns the list of functionality for the "
	     "Client, in a mask made up of the ob.Func_ constants."),
    ATTRMETH(visible,
	     "ob.visible() -- Returns if the client is currently visible "
	     "or hidden."),
    ATTRMETH(decorationSize,
	     "c.decorationSize() -- Returns the size of the Client's "
	     "decorations around the Client window, in a tuple. The format of "
	     "the tuple is (left, top, right, bottom)."),
    ATTRMETH(normal,
	     "c.normal() -- Returns if the window should be treated as a "
	     "normal window. Some windows (desktops, docks, splash screens) "
	     "should have special rules applied to them in a number of "
	     "places regarding focus or user interaction."),
    ATTRMETH(setVisible,
	     "c.setVisible(show) -- Shows or hides the Client."),
    ATTRMETH(raiseWindow,
	     "c.raiseWindow() -- Raises the Client to the top of its layer."),
    ATTRMETH(lowerWindow,
	     "c.lowerWindow() -- Lowers the Client to the bottom of its "
	     "layer."),
    ATTRMETH(focus,
	     "c.focus() -- Focuses the Client. Returns 1 if the Client will "
	     "be focused, or 0 if it will not."),
    ATTRMETH(unfocus,
	     "c.unfocus() -- Unfocuses the Client, leaving nothing focused."),
    ATTRMETH(move,
	     "c.move(x, y) -- Moves the Client to the specified position. The "
	     "top left corner of the Client's decorations is positioned at "
	     "the given x, y."),
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
     return PyString_FromFormat("0x%x", (guint)self->client->window);
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
    w2 = self->client->window;
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
 
   External methods
 
 ***************************************************************************/

void clientwrap_startup()
{
    ClientWrapType.ob_type = &PyType_Type;
    ClientWrapType.tp_methods = ClientWrapMethods;
    PyType_Ready(&ClientWrapType);
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
