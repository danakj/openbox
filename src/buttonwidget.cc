// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "buttonwidget.hh"

namespace ob {

OBButtonWidget::OBButtonWidget(otk::OtkWidget *parent,
                               OBWidget::WidgetType type)
  : otk::OtkWidget(parent),
    OBWidget(type),
    _pressed(false),
    _button(0)
{
}


OBButtonWidget::~OBButtonWidget()
{
}


void OBButtonWidget::setTextures()
{
  switch (type()) {
  case Type_LeftGrip:
  case Type_RightGrip:
    if (_focused)
      setTexture(_style->getGripFocus());
    else
      setTexture(_style->getGripUnfocus());
    break;
  case Type_StickyButton:
  case Type_CloseButton:
  case Type_MaximizeButton:
  case Type_IconifyButton:
    if (_pressed) {
      if (_focused)
        setTexture(_style->getButtonPressedFocus());
      else
        setTexture(_style->getButtonPressedUnfocus());
    } else {
      if (_focused)
        setTexture(_style->getButtonFocus());
      else
        setTexture(_style->getButtonUnfocus());
    }
    break;
  default:
    assert(false); // there's no other button widgets!
  }
}


void OBButtonWidget::setStyle(otk::Style *style)
{
  otk::OtkWidget::setStyle(style);
  setTextures();

  switch (type()) {
  case Type_LeftGrip:
  case Type_RightGrip:
    setBorderColor(_style->getBorderColor());
    break;
  case Type_StickyButton:
  case Type_CloseButton:
  case Type_MaximizeButton:
  case Type_IconifyButton:
    break;
  default:
    assert(false); // there's no other button widgets!
  }
}


void OBButtonWidget::focus()
{
  otk::OtkWidget::focus();
  setTextures();
}


void OBButtonWidget::unfocus()
{
  otk::OtkWidget::unfocus();
  setTextures();
}


void OBButtonWidget::adjust()
{
  // XXX: adjust shit
}


void OBButtonWidget::buttonPressHandler(const XButtonEvent &e)
{
  OtkWidget::buttonPressHandler(e);
  if (_button) return;
  _button = e.button;
  _pressed = true;
  setTextures();
  update();
}


void OBButtonWidget::buttonReleaseHandler(const XButtonEvent &e)
{
  OtkWidget::buttonPressHandler(e);
  if (e.button != _button) return;
  _button = 0;
  _pressed = false;
  setTextures();
  update();
}

}
