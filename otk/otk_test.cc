// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "application.hh"
#include "appwidget.hh"
#include "label.hh"
#include "button.hh"

int main(int argc, char **argv) {
  otk::Application app(argc, argv);

  otk::AppWidget foo(&app, otk::Widget::Vertical, 3);
  otk::Label lab(&foo);
  otk::Label lab2(&foo);
  otk::Button but(&foo);
  otk::Button but2(&foo);
  
  foo.resize(otk::Size(100, 150));

  lab.setText("Hi, I'm a sexy\nlabel!!!");
  lab.setMaxSize(otk::Size(0,0));
  lab2.setText("Me too!!");
  lab2.setBorderWidth(10);
  lab2.setBorderColor(otk::RenderStyle::style(app.screen())->buttonFocusColor());
  but.setText("Im not the default button...");
  but2.setText("But I AM!!");
  but2.setHighlighted(true);
  

  foo.show();

  app.run();

  return 0;
}
