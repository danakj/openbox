// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __appwidget_hh
#define __appwidget_hh

#include "widget.hh"

namespace otk {

class Application;

class AppWidget : public Widget {

public:
  AppWidget(Application *app, Direction direction = Horizontal, int bevel = 0);
  virtual ~AppWidget();

  virtual void show();
  virtual void hide();

  virtual void clientMessageHandler(const XClientMessageEvent &e);
  
private:

  Application *_application;
};

}

#endif // __appwidget_hh
