// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module otk

%{
#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "otk.hh"
%}

%include "stl.i"
//%include std_list.i
%include "ustring.i"

%ignore otk::display;
%inline %{
  otk::Display *Display_instance() { return otk::display; }
%};

%ignore otk::Property::atoms;
%inline %{
  const otk::Atoms& Property_atoms() { return otk::Property::atoms; }
%};

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
%include "rect.hh"
%include "rendercolor.hh"
%include "rendertexture.hh"
%include "font.hh"
%include "renderstyle.hh"
%include "widget.hh"
%include "label.hh"
%include "focuswidget.hh"
%include "focuslabel.hh"
%include "appwidget.hh"
%include "application.hh"
%include "assassin.hh"
%include "button.hh"
//%include "configuration.hh"
%include "display.hh"
%include "rendercontrol.hh"
%include "property.hh"
%include "screeninfo.hh"
%include "strut.hh"
%include "timer.hh"
%include "util.hh"

// for Mod1Mask etc
%include "X11/X.h"
