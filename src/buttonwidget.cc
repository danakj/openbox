// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "buttonwidget.hh"

namespace ob {

OBButtonWidget::OBButtonWidget(otk::OtkWidget *parent,
                               OBWidget::WidgetType type)
  : otk::OtkButton(parent),
    OBWidget(type)
{
}


OBButtonWidget::~OBButtonWidget()
{
}


void OBButtonWidget::setStyle(otk::Style *style)
{
  otk::OtkButton::setStyle(style);

  switch (type()) {
  case Type_LeftGrip:
  case Type_RightGrip:
    setTexture(style->getGripFocus());
    setUnfocusTexture(style->getGripUnfocus());
    setPressedFocusTexture(style->getGripFocus());
    setPressedUnfocusTexture(style->getGripUnfocus());
    setTexture(style->getGripFocus());
    setUnfocusTexture(style->getGripUnfocus());
    setPressedFocusTexture(style->getGripFocus());
    setPressedUnfocusTexture(style->getGripUnfocus());
    setBorderColor(_style->getBorderColor());
    setUnfocusBorderColor(style->getBorderColor());
    break;
  default:
    break;
  }
}


void OBButtonWidget::adjust()
{
  otk::OtkButton::adjust();

  // XXX: adjust shit
}


}
