// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "appwidget.hh"
#include "application.hh"
#include "property.hh"
#include "renderstyle.hh"
#include "display.hh"

extern "C" {
#include <X11/Xlib.h>
}

namespace otk {

AppWidget::AppWidget(Application *app, Direction direction, int bevel)
  : Widget(app->screen(), app, direction, bevel),
    _application(app)
{
  assert(app);

  // set WM Protocols on the window
  Atom protocols[2];
  protocols[0] = Property::atoms.wm_protocols;
  protocols[1] = Property::atoms.wm_delete_window;
  XSetWMProtocols(**display, window(), protocols, 2);
}

AppWidget::~AppWidget()
{
}

void AppWidget::show()
{
  if (!visible())
  _application->_appwidget_count++;
  Widget::show(true);
}

void AppWidget::hide()
{
  if (visible())
    _application->_appwidget_count--;
  Widget::hide();
}

void AppWidget::clientMessageHandler(const XClientMessageEvent &e)
{
  EventHandler::clientMessageHandler(e);
  if (e.message_type == Property::atoms.wm_protocols &&
      static_cast<Atom>(e.data.l[0]) == Property::atoms.wm_delete_window)
    hide();
}

}
