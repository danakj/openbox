#include "application.hh"
#include "eventhandler.hh"
#include "widget.hh"

extern "C" {
#include <X11/Xlib.h>
  
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
}

#include <iostream>

namespace otk {

OtkApplication::OtkApplication(int argc, char **argv)
  : OtkEventDispatcher(), _main_widget(0), _dockable(false)
{
  argc = argc;
  argv = argv;

  OBDisplay::initialize(0);
  const ScreenInfo *s_info = OBDisplay::screenInfo(DefaultScreen(OBDisplay::display));

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
  if (!_main_widget) {
    std::cerr << "ERROR: No main widget set. You must create a main " <<
      "OtkWidget for the OtkApplication before calling " <<
      "OtkApplication::exec().\n";
    ::exit(1);
  }
  while (1) {
    dispatchEvents();
    _timer_manager->fire(); // fire pending events
  }
}

bool OtkApplication::setMainWidget(const OtkWidget *main_widget)
{
  // ignore it if it has already been set
  if (_main_widget) {
    std::cerr << "WARNING: More than one main OtkWidget being created for " <<
      "the OtkApplication!\n";
    return false;
  }

  _main_widget = main_widget;

  // set WM Protocols on the window
  Atom protocols[2];
  protocols[0] = XInternAtom(OBDisplay::display, "WM_PROTOCOLS", false);
  protocols[1] = XInternAtom(OBDisplay::display, "WM_DELETE_WINDOW", false);
  XSetWMProtocols(OBDisplay::display, _main_widget->getWindow(), protocols, 2);

  return true;
}

}
