#include "focuswidget.hh"
#include "button.hh"
#include "display.hh"
#include "configuration.hh"
#include "timerqueuemanager.hh"
#include "image.hh"
#include "style.hh"
#include <iostream>

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

  foo.resize(600, 500);
  foo.setTexture(my_style->getTitleFocus());
  foo.setUnfocusTexture(my_style->getTitleUnfocus());

  foo.setBevelWidth(2);
  foo.setDirection(otk::OtkWidget::Horizontal);

  otk::OtkFocusWidget left(&foo);
  otk::OtkFocusWidget right(&foo);

  left.setDirection(otk::OtkWidget::Horizontal);
  left.setStretchableVert(true);
  left.setStretchableHorz(true);
  left.setTexture(my_style->getTitleFocus());
  left.setUnfocusTexture(my_style->getTitleUnfocus());
 
  right.setDirection(otk::OtkWidget::Vertical);
  right.setBevelWidth(10);
  right.setStretchableVert(true);
  right.setWidth(300);
  right.setTexture(my_style->getTitleFocus());
  right.setUnfocusTexture(my_style->getTitleUnfocus());

  otk::OtkButton iconb(&left);
  otk::OtkFocusWidget label(&left);
  otk::OtkButton maxb(&left);
  otk::OtkButton closeb(&left);
  
  // fixed size
  iconb.setText("foo");
  iconb.press();

  // fix width to 60 and let the height be calculated by its parent
  //label.setHeight(20);
  label.setStretchableVert(true);
  label.setStretchableHorz(true);
  label.setTexture(my_style->getLabelFocus());
  label.setUnfocusTexture(my_style->getLabelUnfocus());

  // fixed size
  maxb.setText("bar");

  // fixed size
  closeb.setText("fuubar");

  otk::OtkFocusWidget rblef(&right);
  otk::OtkButton rbutt1(&right);
  otk::OtkButton rbutt2(&right);

  rblef.setStretchableHorz(true);
  rblef.setHeight(50);
  rblef.setTexture(my_style->getHandleFocus());
  rblef.setUnfocusTexture(my_style->getHandleUnfocus());
  
  rbutt1.setText("this is fucking tight");
  rbutt2.setText("heh, WOOP");

  // will recursively unfocus its children
  //foo.unfocus();
  foo.update();
  foo.show();

  while (1) {
    if (XPending(otk::OBDisplay::display)) {
      XEvent e;
      XNextEvent(otk::OBDisplay::display, &e);
      if (e.type == Expose) {
        foo.expose(e.xexpose);
      } else if (e.type == ConfigureNotify) {
        foo.configure(e.xconfigure);
      }
    } 
  }

  delete my_style;
  delete tm;
  delete ctrl;

  otk::OBDisplay::destroy();

  return 0;
}
