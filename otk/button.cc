// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "button.hh"

namespace otk {

Button::Button(Widget *parent)
  : FocusLabel(parent), _pressed(false), _pressed_focus_tx(0),
    _pressed_unfocus_tx(0), _unpr_focus_tx(0), _unpr_unfocus_tx(0)
{
}

Button::~Button()
{
}


void Button::setStyle(Style *style)
{
  FocusLabel::setStyle(style);
  
  setTexture(style->getButtonFocus());
  setUnfocusTexture(style->getButtonUnfocus());
  _pressed_focus_tx = style->getButtonPressedFocus();
  _pressed_unfocus_tx = style->getButtonPressedUnfocus();
}


void Button::press(unsigned int mouse_button)
{
  if (_pressed) return;

  if (_pressed_focus_tx)
    FocusWidget::setTexture(_pressed_focus_tx);
  if (_pressed_unfocus_tx)
    FocusWidget::setUnfocusTexture(_pressed_unfocus_tx);
  _pressed = true;
  _mouse_button = mouse_button;
}

void Button::release(unsigned int mouse_button)
{
  if (_mouse_button != mouse_button) return; // wrong button

  FocusWidget::setTexture(_unpr_focus_tx);
  FocusWidget::setUnfocusTexture(_unpr_unfocus_tx);
  _pressed = false;
}

void Button::setTexture(Texture *texture)
{
  FocusWidget::setTexture(texture);
  _unpr_focus_tx = texture;
}

void Button::setUnfocusTexture(Texture *texture)
{
  FocusWidget::setUnfocusTexture(texture);
  _unpr_unfocus_tx = texture;
}

void Button::buttonPressHandler(const XButtonEvent &e)
{
  press(e.button);
  update();
  FocusWidget::buttonPressHandler(e);
}

void Button::buttonReleaseHandler(const XButtonEvent &e)
{
  release(e.button);
  update();
  FocusWidget::buttonReleaseHandler(e);
}

}
