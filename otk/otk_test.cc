#include "application.hh"
#include "focuswidget.hh"
#include "appwidget.hh"
#include "button.hh"

int main(int argc, char **argv) {
  otk::OtkApplication app(argc, argv);

  otk::OtkAppWidget foo(&app);

  foo.resize(600, 500);
  foo.setTexture(app.getStyle()->getTitleFocus());
//  foo.setUnfocusTexture(app.getStyle()->getTitleUnfocus());

  foo.setBevelWidth(2);
  foo.setDirection(otk::OtkWidget::Horizontal);

  otk::OtkFocusWidget left(&foo);
  otk::OtkFocusWidget right(&foo);

  left.setDirection(otk::OtkWidget::Horizontal);
  left.setStretchableVert(true);
  left.setStretchableHorz(true);
  left.setTexture(app.getStyle()->getTitleFocus());
  left.setUnfocusTexture(app.getStyle()->getTitleUnfocus());
 
  right.setDirection(otk::OtkWidget::Vertical);
  right.setBevelWidth(10);
  right.setStretchableVert(true);
  right.setWidth(300);
  right.setTexture(app.getStyle()->getTitleFocus());
  right.setUnfocusTexture(app.getStyle()->getTitleUnfocus());

  otk::OtkButton iconb(&left);
  iconb.resize(40,20);
  otk::OtkFocusWidget label(&left);
  otk::OtkButton maxb(&left);
  otk::OtkButton closeb(&left);
  
  // fixed size
  iconb.setText("foo");
  iconb.press(Button1);

  // fix width to 60 and let the height be calculated by its parent
  //label.setHeight(20);
  label.setStretchableVert(true);
  label.setStretchableHorz(true);
  label.setTexture(app.getStyle()->getLabelFocus());
  label.setUnfocusTexture(app.getStyle()->getLabelUnfocus());

  // fixed size
  maxb.setText("bar");

  // fixed size
  closeb.setText("fuubar");

  otk::OtkFocusWidget rblef(&right);
  otk::OtkButton rbutt1(&right);
  otk::OtkButton rbutt2(&right);

  rblef.setStretchableHorz(true);
  rblef.setHeight(50);
  rblef.setTexture(app.getStyle()->getHandleFocus());
  rblef.setUnfocusTexture(app.getStyle()->getHandleUnfocus());
  
  rbutt1.setText("this is fucking tight");
  rbutt2.setText("heh, WOOP");

  // will recursively unfocus its children
  //foo.unfocus();

  foo.show();

  app.exec();

  return 0;
}
