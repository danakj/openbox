// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "backgroundwidget.hh"

namespace ob {

OBBackgroundWidget::OBBackgroundWidget(otk::OtkWidget *parent,
                                       OBWidget::WidgetType type)
  : otk::OtkFocusWidget(parent),
    OBWidget(type)
{
}


OBBackgroundWidget::~OBBackgroundWidget()
{
}


void OBBackgroundWidget::setStyle(otk::Style *style)
{
  switch (type()) {
  case Type_Titlebar:
    setTexture(style->getTitleFocus());
    setUnfocusTexture(style->getTitleUnfocus());
    setBorderColor(style->getBorderColor());
    break;
  case Type_Handle:
    setTexture(style->getHandleFocus());
    setUnfocusTexture(style->getHandleUnfocus());
    setBorderColor(style->getBorderColor());
    break;
  case Type_Plate:
    setBorderColor(&style->getFrameFocus()->color());
    setUnfocusBorderColor(&style->getFrameUnfocus()->color());
    break;
  default:
    assert(false); // there's no other background widgets!
  }

  otk::OtkFocusWidget::setStyle(style);
}


void OBBackgroundWidget::adjust()
{
  otk::OtkFocusWidget::adjust();

  // XXX: adjust shit
}


}
