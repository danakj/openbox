// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "appwidget.hh"
#include "application.hh"
#include "property.hh"

extern "C" {
#include <X11/Xlib.h>
}

namespace otk {

AppWidget::AppWidget(Application *app, Direction direction,
                     Cursor cursor, int bevel_width)
  : Widget(app, app->getStyle(), direction, cursor, bevel_width),
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

void AppWidget::show(void)
{
  Widget::show(true);

  _application->_appwidget_count++;
}

void AppWidget::hide(void)
{
  Widget::hide();

  _application->_appwidget_count--;
}

void AppWidget::clientMessageHandler(const XClientMessageEvent &e)
{
  EventHandler::clientMessageHandler(e);
  if (e.message_type == Property::atoms.wm_protocols &&
      static_cast<Atom>(e.data.l[0]) == Property::atoms.wm_delete_window)
    hide();
}

}
