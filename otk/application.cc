// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "application.hh"
#include "eventhandler.hh"
#include "timer.hh"
#include "property.hh"
#include "rendercolor.hh"
#include "renderstyle.hh"

extern "C" {
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
}

#include <iostream>

namespace otk {

Application::Application(int argc, char **argv)
  : EventDispatcher(),
    _display(),
    _dockable(false),
    _appwidget_count(0)
{
  (void)argc;
  (void)argv;

  _screen = DefaultScreen(*_display);
  
  Timer::initialize();
  RenderColor::initialize();
  RenderStyle::initialize();
  Property::initialize();

  loadStyle();
}

Application::~Application()
{
  RenderStyle::destroy();
  RenderColor::destroy();
  Timer::destroy();
}

void Application::loadStyle(void)
{
  // XXX: find the style name as a property
  std::string style = "/usr/local/share/openbox/styles/artwiz";
  //_style->load(style);
}

void Application::run(void)
{
  if (_appwidget_count <= 0) {
    std::cerr << "ERROR: No main widgets exist. You must create and show() " <<
      "an AppWidget for the Application before calling " <<
      "Application::run().\n";
    ::exit(1);
  }

  while (_appwidget_count > 0) {
    dispatchEvents();
    if (_appwidget_count <= 0)
      break;
    Timer::dispatchTimers(); // fire pending events
  }
}

}
