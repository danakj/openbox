// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "rootwindow.hh"
#include "openbox.hh"
#include "screen.hh"
#include "client.hh"
#include "otk/display.hh"

namespace ob {

OBRootWindow::OBRootWindow(int screen)
  : OBWidget(OBWidget::Type_Root),
    _info(otk::OBDisplay::screenInfo(screen))
{
  updateDesktopNames();

  Openbox::instance->registerHandler(_info->rootWindow(), this);
}


OBRootWindow::~OBRootWindow()
{
}





}
