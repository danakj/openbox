#include "application.hh"
#include "eventhandler.hh"
#include "widget.hh"

extern "C" {
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
}

#include <iostream>

namespace otk {

OtkApplication::OtkApplication(int argc, char **argv)
  : OtkEventDispatcher(),
    _dockable(false),
    _appwidget_count(0)
{
  argc = argc;
  argv = argv;

  OBDisplay::initialize(0);
  const ScreenInfo *s_info =
    OBDisplay::screenInfo(DefaultScreen(OBDisplay::display));

  _timer_manager = new OBTimerQueueManager();
  _img_ctrl = new BImageControl(_timer_manager, s_info, True, 4, 5, 200);
  _style_conf = new Configuration(False);
  _style = new Style(_img_ctrl);

  loadStyle();
}

OtkApplication::~OtkApplication()
{
  delete _style_conf;
  delete _img_ctrl;
  delete _timer_manager;
  delete _style;
  
  OBDisplay::destroy();
}

void OtkApplication::loadStyle(void)
{
  // find the style name as a property
  std::string style = "/usr/local/share/openbox/styles/artwiz";
  _style_conf->setFile(style);
  if (!_style_conf->load()) {
    std::cerr << "Unable to load style \"" << style << "\". Aborting.\n";
    ::exit(1);
  }
  _style->load(*_style_conf);
}

void OtkApplication::exec(void)
{
  if (_appwidget_count <= 0) {
    std::cerr << "ERROR: No main widgets exist. You must create and show() " <<
      "an OtkAppWidget for the OtkApplication before calling " <<
      "OtkApplication::exec().\n";
    ::exit(1);
  }

  while (_appwidget_count > 0) {
    dispatchEvents();
    _timer_manager->fire(); // fire pending events
  }
}

}
