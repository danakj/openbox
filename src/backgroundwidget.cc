// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "backgroundwidget.hh"

namespace ob {

OBBackgroundWidget::OBBackgroundWidget(otk::OtkWidget *parent,
                                       OBWidget::WidgetType type)
  : otk::OtkWidget(parent),
    OBWidget(type)
{
}


OBBackgroundWidget::~OBBackgroundWidget()
{
}


void OBBackgroundWidget::setTextures()
{
  switch (type()) {
  case Type_Titlebar:
    if (_focused)
      setTexture(_style->getTitleFocus());
    else
      setTexture(_style->getTitleUnfocus());
    break;
  case Type_Handle:
    if (_focused)
      setTexture(_style->getHandleFocus());
    else
      setTexture(_style->getHandleUnfocus());
    break;
  case Type_Plate:
    if (_focused)
      setBorderColor(&_style->getFrameFocus()->color());
    else
      setBorderColor(&_style->getFrameUnfocus()->color());
    break;
  default:
    assert(false); // there's no other background widgets!
  }
}


void OBBackgroundWidget::setStyle(otk::Style *style)
{
  OtkWidget::setStyle(style);
  setTextures();
  switch (type()) {
  case Type_Titlebar:
  case Type_Handle:
    setBorderColor(_style->getBorderColor());
    break;
  case Type_Plate:
    break;
  default:
    assert(false); // there's no other background widgets!
  }
}


void OBBackgroundWidget::focus()
{
  otk::OtkWidget::focus();
  setTextures();
}


void OBBackgroundWidget::unfocus()
{
  otk::OtkWidget::unfocus();
  setTextures();
}


void OBBackgroundWidget::adjust()
{
  // nothing to adjust here. its done in OBFrame::adjustSize
}

}
