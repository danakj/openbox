// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module otk

%import "eventhandler.hh"
%import "eventdispatcher.hh"

%include "otk_strut.i"
%include "otk_point.i"
%include "otk_size.i"
%include "otk_rect.i"
%include "otk_rendercolor.i"
%include "otk_rendertexture.i"
%include "otk_font.i"
%include "otk_ustring.i"
%include "otk_renderstyle.i"
%include "otk_widget.i"
%include "otk_label.i"
%include "otk_button.i"
%include "otk_application.i"
%include "otk_appwidget.i"
%include "otk_property.i"
%include "otk_timer.i"

%immutable otk::Property::atoms;

%ignore TimerLessThan;


// for Window etc
%import "X11/X.h"

// globals
%pythoncode %{
atoms = cvar.Property_atoms;

def style(screen):
    return RenderStyle_style(screen)

%}
