// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "../config.h"
#include "rect.h"

PyObject *OtkRect_New(int x, int y, int width, int height)
{
  OtkRect* self = PyObject_New(OtkRect, &OtkRect_Type);

  self->x = x;
  self->y = y;
  self->width = width;
  self->height = height;

  return (PyObject*)self;
}



static PyObject *otkrect_getx(OtkRect *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, ":getX"))
    return NULL;
  return PyInt_FromLong(self->x);
}

static PyObject *otkrect_gety(OtkRect *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, ":getY"))
    return NULL;
  return PyInt_FromLong(self->y);
}

static PyObject *otkrect_getwidth(OtkRect *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, ":getWidth"))
    return NULL;
  return PyInt_FromLong(self->width);
}

static PyObject *otkrect_getheight(OtkRect *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, ":getHeight"))
    return NULL;
  return PyInt_FromLong(self->height);
}


static PyMethodDef get_methods[] = {
  {"getX", (PyCFunction)otkrect_getx, METH_VARARGS,
   "Get the X coordinate."},
  {"getY", (PyCFunction)otkrect_gety, METH_VARARGS,
   "Get the Y coordinate."},
  {"getWidth", (PyCFunction)otkrect_getwidth, METH_VARARGS,
   "Get the width."},
  {"getHeight", (PyCFunction)otkrect_getheight, METH_VARARGS,
   "Get the height."},
  {NULL, NULL, 0, NULL}
};



static void otkrect_dealloc(PyObject *self)
{
  PyObject_Del(self);
}

static PyObject *otkrect_getattr(PyObject *obj, char *name)
{
  return Py_FindMethod(get_methods, obj, name);
}


PyTypeObject OtkRect_Type = {
  PyObject_HEAD_INIT(NULL)
  0,
  "OtkRect",
  sizeof(OtkRect),
  0,
  otkrect_dealloc,     /*tp_dealloc*/
  0,                   /*tp_print*/
  otkrect_getattr,     /*tp_getattr*/
  0,                   /*tp_setattr*/
  0,                   /*tp_compare*/
  0,                   /*tp_repr*/
  0,                   /*tp_as_number*/
  0,                   /*tp_as_sequence*/
  0,                   /*tp_as_mapping*/
  0,                   /*tp_hash */
};
