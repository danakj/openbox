// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "backgroundwidget.hh"

namespace ob {

BackgroundWidget::BackgroundWidget(otk::Widget *parent,
                                   WidgetBase::WidgetType type)
  : otk::Widget(parent),
    WidgetBase(type)
{
}


BackgroundWidget::~BackgroundWidget()
{
}


void BackgroundWidget::setTextures()
{
  switch (type()) {
  case Type_Titlebar:
    if (_focused)
      setTexture(_style->titlebarFocusBackground());
    else
      setTexture(_style->titlebarUnfocusBackground());
    break;
  case Type_Handle:
    if (_focused)
      setTexture(_style->handleFocusBackground());
    else
      setTexture(_style->handleUnfocusBackground());
    break;
  case Type_Plate:
    if (_focused)
      setBorderColor(_style->clientBorderFocusColor());
    else
      setBorderColor(_style->clientBorderUnfocusColor());
    break;
  default:
    assert(false); // there's no other background widgets!
  }
}


void BackgroundWidget::setStyle(otk::RenderStyle *style)
{
  Widget::setStyle(style);
  setTextures();
  switch (type()) {
  case Type_Titlebar:
  case Type_Handle:
    setBorderColor(_style->frameBorderColor());
    break;
  case Type_Plate:
    break;
  default:
    assert(false); // there's no other background widgets!
  }
}


void BackgroundWidget::focus()
{
  otk::Widget::focus();
  setTextures();
}


void BackgroundWidget::unfocus()
{
  otk::Widget::unfocus();
  setTextures();
}


void BackgroundWidget::adjust()
{
  // nothing to adjust here. its done in Frame::adjustSize
}

}
