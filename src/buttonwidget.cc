// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "buttonwidget.hh"
#include "otk/gccache.hh" // otk::BPen

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


void OBButtonWidget::update()
{
  otk::PixmapMask *pm;
  int width;
  bool draw = _dirty;

  otk::OtkWidget::update();

  if (draw) {
    switch (type()) {
    case Type_StickyButton:
      pm = _style->getStickyButtonMask();
      break;
    case Type_CloseButton:
      pm = _style->getCloseButtonMask();
      break;
    case Type_MaximizeButton:
      pm = _style->getMaximizeButtonMask();
      break;
    case Type_IconifyButton:
      pm = _style->getIconifyButtonMask();
      break;
    case Type_LeftGrip:
    case Type_RightGrip:
      return; // no drawing
    default:
      assert(false); // there's no other button widgets!
    }

    if (pm->mask == None) return; // no mask for the button, leave it empty

    width = _rect.width();
  
    otk::BPen pen(_focused ? *_style->getButtonPicFocus() :
                  *_style->getButtonPicUnfocus());

    // set the clip region
    XSetClipMask(otk::OBDisplay::display, pen.gc(), pm->mask);
    XSetClipOrigin(otk::OBDisplay::display, pen.gc(),
                   (width - pm->w)/2, (width - pm->h)/2);

    // fill in the clipped region
    XFillRectangle(otk::OBDisplay::display, _window, pen.gc(),
                   (width - pm->w)/2, (width - pm->h)/2,
                   (width + pm->w)/2, (width + pm->h)/2);

    // unset the clip region
    XSetClipMask(otk::OBDisplay::display, pen.gc(), None);
    XSetClipOrigin(otk::OBDisplay::display, pen.gc(), 0, 0);
  }
}


void OBButtonWidget::adjust()
{
  // nothing to adjust. no children.
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
