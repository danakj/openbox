// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "application.hh"
#include "eventhandler.hh"
#include "timer.hh"
#include "property.hh"
#include "rendercolor.hh"
#include "renderstyle.hh"
#include "display.hh"

#include <cstdlib>
#include <iostream>

namespace otk {

extern void initialize();
extern void destroy();

Application::Application(int argc, char **argv)
  : EventDispatcher(),
    _dockable(false),
    _appwidget_count(0)
{
  (void)argc;
  (void)argv;

  otk::initialize();
  
  _screen = DefaultScreen(**display);
  
  loadStyle();
}

Application::~Application()
{
  otk::destroy();
}

void Application::loadStyle(void)
{
  // XXX: find the style name as a property
  std::string style = "/usr/local/share/openbox/styles/artwiz";
  //_style->load(style);
  otk::RenderStyle::setStyle(_screen, style);
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
    if (_appwidget_count > 0)
      Timer::dispatchTimers(); // fire pending events
  }
}

}
