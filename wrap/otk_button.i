// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module otk_button

%{
#include "config.h"
#include "button.hh"
%}

%include "otk_widget.i"
%include "otk_ustring.i"
%include "otk_font.i"
%include "otk_renderstyle.i"

namespace otk {

%ignore Button::clickHandler(unsigned int);
%ignore Button::styleChanged(const RenderStyle &);

}

%include "button.hh"
