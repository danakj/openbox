// SWIG typemaps for otk::ustring

%{
#include "otk/ustring.hh"
%}

namespace otk {

    class ustring;

    /* Overloading check */

    %typemap(typecheck) ustring = char *;
    %typemap(typecheck) const ustring & = char *;

    %typemap(in) ustring {
        if (PyString_Check($input))
            $1 = otk::ustring(PyString_AsString($input));
        else
            SWIG_exception(SWIG_TypeError, "string expected");
    }

    %typemap(in) const ustring & (otk::ustring temp) {
        if (PyString_Check($input)) {
            temp = otk::ustring(PyString_AsString($input));
            $1 = &temp;
        } else {
            SWIG_exception(SWIG_TypeError, "string expected");
        }
    }

    %typemap(out) ustring {
        $result = PyString_FromString($1.c_str());
    }

    %typemap(out) const ustring & {
        $result = PyString_FromString($1->c_str());
    }

}
