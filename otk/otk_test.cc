#include "focuswidget.hh"
#include "button.hh"
#include "display.hh"
#include "configuration.hh"
#include "timerqueuemanager.hh"
#include "image.hh"
#include "style.hh"

int main(void) {
  otk::OBDisplay::initialize(NULL);
  otk::Configuration style_conf(False);
  otk::OBTimerQueueManager *tm = new otk::OBTimerQueueManager();
  const otk::ScreenInfo *s_info =
    otk::OBDisplay::screenInfo(DefaultScreen(otk::OBDisplay::display));
  otk::BImageControl *ctrl = new otk::BImageControl(tm, s_info, True, 4, 5, 200);

  otk::Style *my_style = new otk::Style(ctrl);

  style_conf.setFile("/usr/local/share/openbox/styles/artwiz");
  style_conf.load();

  my_style->load(style_conf);

  otk::OtkFocusWidget foo(my_style);
  otk::OtkButton iconb(&foo);
  otk::OtkFocusWidget label(&foo);
  otk::OtkButton maxb(&foo);
  otk::OtkButton closeb(&foo);

  foo.setBevelWidth(2);
  foo.setDirection(otk::OtkWidget::Vertical);
  
  foo.setHeight(400);
  foo.setTexture(my_style->getTitleFocus());
  foo.setUnfocusTexture(my_style->getTitleUnfocus());

  // fixed size
  iconb.setText("foo");
  iconb.press();

  // fix width to 60 and let the height be calculated by its parent
  label.setWidth(60);
  label.setStretchableVert(true);
  label.setTexture(my_style->getLabelFocus());
  label.setUnfocusTexture(my_style->getLabelUnfocus());

  // fixed size
  maxb.setText("bar");

  // fixed size
  closeb.setText("fuubar");

  // will recursively unfocus its children
  //foo.unfocus();
  foo.update();
  foo.show();

  while (1) {
    if (XPending(otk::OBDisplay::display)) {
      XEvent e;
      XNextEvent(otk::OBDisplay::display, &e);
      if (e.type == Expose)
        foo.expose(e.xexpose);
    } 
  }

  delete my_style;
  delete tm;
  delete ctrl;

  otk::OBDisplay::destroy();

  return 0;
}
