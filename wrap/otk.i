// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module otk

%{
#include "otk.hh"
%}

%include "stl.i"
//%include std_list.i
%include "ustring.i"

%immutable otk::display;
%immutable otk::Property::atoms;

namespace otk {
/*%rename(setValue_bool) Configuration::setValue(std::string const &,bool);
%rename(setValue_unsigned) Configuration::setValue(const std::string &, unsigned int);
%rename(setValue_long) Configuration::setValue(const std::string &, long);
%rename(setValue_unsignedlong) Configuration::setValue(const std::string &, unsigned long);
%rename(setValue_string) Configuration::setValue(const std::string &, const std::string &);
%rename(setValue_charptr) Configuration::setValue(const std::string &, const char *);*/

%rename(itostring_unsigned) itostring(unsigned int);
%rename(itostring_long) itostring(long);
%rename(itostring_unsigned_long) itostring(unsigned long);

// these are needed for guile, but not needed for python!
//%rename(equals) BColor::operator==;
//%rename(equals) Rect::operator==;
//%rename(equals) BTexture::operator==;
//%ignore BColor::operator!=;
//%ignore BTexture::operator!=;
%ignore Rect::operator!=;
%ignore Rect::operator|;
%ignore Rect::operator|=;
%ignore Rect::operator&;
%ignore Rect::operator&=;
//%ignore OBTimer::operator<;
%ignore TimerLessThan;

/*
%rename(set_multi) OtkProperty::set(Window, Atoms, Atoms, unsigned long[], int);
%rename(set_string) OtkProperty::set(Window, Atoms, StringType, const std::string &);
%rename(set_string_multi) OtkProperty::set(Window, Atoms, StringType, const StringVect &);
*/
}

%include "eventhandler.hh"
%include "eventdispatcher.hh"
%include "point.hh"
%include "size.hh"
%include "rect.hh"
%include "rendercolor.hh"
%include "rendertexture.hh"
%include "font.hh"
%include "renderstyle.hh"
%include "widget.hh"
%include "label.hh"
%include "appwidget.hh"
%include "application.hh"
%include "assassin.hh"
%include "button.hh"
%include "display.hh"
%include "rendercontrol.hh"
%include "property.hh"
%include "screeninfo.hh"
%include "strut.hh"

// for Window etc
%import "X11/X.h"

// globals
%pythoncode %{
display = cvar.display;
atoms = cvar.Property_atoms;

def style(screen):
    return RenderStyle_style(screen)

%}
