// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module otk_widget

%{
#include "config.h"
#include "widget.hh"
%}

%include "otk_rendercolor.i"

%typemap(python,out) const std::list<Widget*>& {
  std::list<Widget*> *v = $1;
  unsigned int s = v->size();
  PyObject *l = PyList_New(s);

  std::list<Widget*>::const_iterator it = v->begin(), end = v->end();
  for (unsigned int i = 0; i < s; ++i, ++it) {
    PyObject *pdata = SWIG_NewPointerObj((void*)*it,SWIGTYPE_p_otk__Widget,0);
    PyList_SET_ITEM(l, i, pdata);
  }
  $result = l;
}

namespace otk {

%ignore Widget::exposeHandler(const XExposeEvent &);
%ignore Widget::configureHandler(const XConfigureEvent &);
%ignore Widget::styleChanged(const RenderStyle &);

}

%import "../otk/eventhandler.hh"
%import "../otk/renderstyle.hh"
%include "widget.hh"
