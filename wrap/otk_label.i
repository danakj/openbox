// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module otk_label

%{
#include "config.h"
#include "label.hh"
%}

%include "otk_widget.i"
%include "otk_ustring.i"
%include "otk_font.i"
%include "otk_renderstyle.i"

namespace otk {

%ignore Label::renderForeground(Surface &);

}

%include "label.hh"
