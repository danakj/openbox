#include "widget.hh"
#include "display.hh"
#include "configuration.hh"
#include "timerqueuemanager.hh"
#include "image.hh"
#include "style.hh"

#include <unistd.h>

int main(void) {
  otk::OBDisplay::initialize(NULL);
  otk::Configuration style_conf(False);
  otk::OBTimerQueueManager *tm = new otk::OBTimerQueueManager();
  const otk::ScreenInfo *s_info =
    otk::OBDisplay::screenInfo(DefaultScreen(otk::OBDisplay::display));
  otk::BImageControl *ctrl = new otk::BImageControl(tm, s_info, True, 4, 5, 200);

  otk::Style *my_style = new otk::Style(0ul, ctrl);

  const char *sfile = "/usr/local/share/openbox/styles/artwiz";
  
  style_conf.setFile(sfile);
  style_conf.load();

  my_style->load(style_conf);

  otk::OtkWidget foo(my_style);
  otk::OtkWidget bar(&foo);
  otk::OtkWidget baz(&foo);
  otk::OtkWidget blef(&bar);

  foo.setTexture(my_style->getButtonFocus());
  foo.setGeometry(0, 0, 100, 110);

  bar.setTexture(my_style->getLabelFocus());
  bar.setGeometry(10, 10, 80, 40);

  baz.setTexture(my_style->getLabelFocus());
  baz.setGeometry(10, 60, 80, 40);

  blef.setTexture(my_style->getHandleFocus());
  blef.setGeometry(10, 10, 60, 20);

  foo.show();

  while (1) {
    if (XPending(otk::OBDisplay::display)) {
      XEvent e;
      XNextEvent(otk::OBDisplay::display, &e);
    } 
  }

  delete my_style;
  delete tm;
  delete ctrl;

  otk::OBDisplay::destroy();

  return 0;
}
