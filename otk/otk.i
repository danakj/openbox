// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

%module otk

%{
#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "application.hh"
#include "appwidget.hh"
#include "assassin.hh"
#include "button.hh"
#include "color.hh"
#include "configuration.hh"
#include "display.hh"
#include "eventdispatcher.hh"
#include "eventhandler.hh"
#include "focuslabel.hh"
#include "focuswidget.hh"
#include "font.hh"
#include "gccache.hh"
#include "image.hh"
#include "label.hh"
#include "point.hh"
#include "property.hh"
#include "rect.hh"
#include "screeninfo.hh"
#include "strut.hh"
#include "style.hh"
#include "texture.hh"
#include "timer.hh"
#include "timerqueue.hh"
#include "timerqueuemanager.hh"
#include "util.hh"
#include "widget.hh"
%}

%include "stl.i"
//%include std_list.i

%ignore otk::OBDisplay::display;
%inline %{
  Display *OBDisplay_display() { return otk::OBDisplay::display; }
%};

namespace otk {
%rename(setValue_bool) Configuration::setValue(std::string const &,bool);
%rename(setValue_unsigned) Configuration::setValue(const std::string &, unsigned int);
%rename(setValue_long) Configuration::setValue(const std::string &, long);
%rename(setValue_unsignedlong) Configuration::setValue(const std::string &, unsigned long);
%rename(setValue_string) Configuration::setValue(const std::string &, const std::string &);
%rename(setValue_charptr) Configuration::setValue(const std::string &, const char *);

%rename(itostring_unsigned) itostring(unsigned int);
%rename(itostring_long) itostring(long);
%rename(itostring_unsigned_long) itostring(unsigned long);

// these are needed for guile, but not needed for python!
%rename(equals) BColor::operator==;
%rename(equals) Rect::operator==;
%rename(equals) BTexture::operator==;
%ignore BColor::operator!=;
%ignore BTexture::operator!=;
%ignore Rect::operator!=;
%ignore Rect::operator|;
%ignore Rect::operator|=;
%ignore Rect::operator&;
%ignore Rect::operator&=;
%ignore OBTimer::operator<;
%ignore TimerLessThan;

/*
%rename(set_multi) OtkProperty::set(Window, Atoms, Atoms, unsigned long[], int);
%rename(set_string) OtkProperty::set(Window, Atoms, StringType, const std::string &);
%rename(set_string_multi) OtkProperty::set(Window, Atoms, StringType, const StringVect &);
*/
}

%include "eventdispatcher.hh"
%include "eventhandler.hh"
%include "widget.hh"
%include "focuswidget.hh"
%include "focuslabel.hh"
%include "appwidget.hh"
%include "application.hh"
%include "assassin.hh"
%include "button.hh"
%include "color.hh"
%include "configuration.hh"
%include "display.hh"
%include "font.hh"
%include "gccache.hh"
%include "image.hh"
%include "label.hh"
%include "point.hh"
%include "property.hh"
%include "rect.hh"
%include "screeninfo.hh"
%include "strut.hh"
%include "style.hh"
%include "texture.hh"
%include "timer.hh"
%include "timerqueue.hh"
%include "timerqueuemanager.hh"
%include "util.hh"

// for Mod1Mask etc
%include "X11/X.h"
