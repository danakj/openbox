// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "buttonwidget.hh"
#include "otk/gccache.hh" // otk::BPen

namespace ob {

ButtonWidget::ButtonWidget(otk::Widget *parent,
                           WidgetBase::WidgetType type)
  : otk::Widget(parent),
    WidgetBase(type),
    _pressed(false),
    _button(0)
{
}


ButtonWidget::~ButtonWidget()
{
}


void ButtonWidget::setTextures()
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


void ButtonWidget::setStyle(otk::Style *style)
{
  otk::Widget::setStyle(style);
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


void ButtonWidget::update()
{
  otk::PixmapMask *pm;
  int width;
  bool draw = _dirty;

  otk::Widget::update();

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
  
    otk::Pen pen(_focused ? *_style->getButtonPicFocus() :
                 *_style->getButtonPicUnfocus());

    // set the clip region
    XSetClipMask(otk::Display::display, pen.gc(), pm->mask);
    XSetClipOrigin(otk::Display::display, pen.gc(),
                   (width - pm->w)/2, (width - pm->h)/2);

    // fill in the clipped region
    XFillRectangle(otk::Display::display, _window, pen.gc(),
                   (width - pm->w)/2, (width - pm->h)/2,
                   (width + pm->w)/2, (width + pm->h)/2);

    // unset the clip region
    XSetClipMask(otk::Display::display, pen.gc(), None);
    XSetClipOrigin(otk::Display::display, pen.gc(), 0, 0);
  }
}


void ButtonWidget::adjust()
{
  // nothing to adjust. no children.
}


void ButtonWidget::focus()
{
  otk::Widget::focus();
  setTextures();
}


void ButtonWidget::unfocus()
{
  otk::Widget::unfocus();
  setTextures();
}


void ButtonWidget::buttonPressHandler(const XButtonEvent &e)
{
  otk::Widget::buttonPressHandler(e);
  if (_button) return;
  _button = e.button;
  _pressed = true;
  setTextures();
  update();
}


void ButtonWidget::buttonReleaseHandler(const XButtonEvent &e)
{
  otk::Widget::buttonPressHandler(e);
  if (e.button != _button) return;
  _button = 0;
  _pressed = false;
  setTextures();
  update();
}

}
