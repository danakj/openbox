// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "application.hh"
#include "eventhandler.hh"
#include "widget.hh"
#include "timer.hh"

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

  const ScreenInfo *s_info = _display.screenInfo(DefaultScreen(*_display));

  Timer::initialize();
  _img_ctrl = new ImageControl(s_info, True, 4, 5, 200);
  _style_conf = new Configuration(False);
  _style = new Style(_img_ctrl);

  loadStyle();
}

Application::~Application()
{
  delete _style_conf;
  delete _img_ctrl;
  delete _style;
  Timer::destroy();
}

void Application::loadStyle(void)
{
  // find the style name as a property
  std::string style = "/usr/local/share/openbox/styles/artwiz";
  _style_conf->setFile(style);
  if (!_style_conf->load()) {
    std::cerr << "ERROR: Unable to load style \"" << style << "\".\n";
    ::exit(1);
  }
  _style->load(*_style_conf);
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
    Timer::dispatchTimers(); // fire pending events
  }
}

}
