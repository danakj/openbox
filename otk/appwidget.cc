// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "appwidget.hh"
#include "application.hh"

extern "C" {
#include <X11/Xlib.h>
}

namespace otk {

OtkAppWidget::OtkAppWidget(OtkApplication *app, Direction direction,
                           Cursor cursor, int bevel_width)
  : OtkWidget(app, app->getStyle(), direction, cursor, bevel_width),
    _application(app)
{
  assert(app);

  _wm_protocols = XInternAtom(OBDisplay::display, "WM_PROTOCOLS", false);
  _wm_delete = XInternAtom(OBDisplay::display, "WM_DELETE_WINDOW", false);

  // set WM Protocols on the window
  Atom protocols[2];
  protocols[0] = _wm_protocols;
  protocols[1] = _wm_delete;
  XSetWMProtocols(OBDisplay::display, window(), protocols, 2);
}

OtkAppWidget::~OtkAppWidget()
{
}

void OtkAppWidget::show(void)
{
  OtkWidget::show(true);

  _application->_appwidget_count++;
}

void OtkAppWidget::hide(void)
{
  OtkWidget::hide();

  _application->_appwidget_count--;
}

void OtkAppWidget::clientMessageHandler(const XClientMessageEvent &e)
{
  OtkEventHandler::clientMessageHandler(e);
  if (e.message_type == _wm_protocols &&
      static_cast<Atom>(e.data.l[0]) == _wm_delete)
    hide();
}

}
