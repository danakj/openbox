// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "button.hh"

namespace otk {

Button::Button(Widget *parent)
  : Label(parent),
    _pressed(false)
{
  setHorizontalJustify(RenderStyle::CenterJustify);
  setVerticalJustify(RenderStyle::CenterJustify);
  styleChanged(*RenderStyle::style(screen()));
}

Button::~Button()
{
}

void Button::press(unsigned int mouse_button)
{
  if (_pressed) return;

  _pressed = true;
  _mouse_button = mouse_button;

  styleChanged(*RenderStyle::style(screen()));
  refresh();
}

void Button::release(unsigned int mouse_button)
{
  if (!_pressed || _mouse_button != mouse_button) return; // wrong button

  _pressed = false;

  styleChanged(*RenderStyle::style(screen()));
  refresh();
}

void Button::buttonPressHandler(const XButtonEvent &e)
{
  Widget::buttonPressHandler(e);
  press(e.button);
}

void Button::buttonReleaseHandler(const XButtonEvent &e)
{
  Widget::buttonReleaseHandler(e);
  release(e.button);
}

void Button::styleChanged(const RenderStyle &style)
{
  if (isHighlighted()) {
    if (_pressed)
      _texture = style.buttonPressFocusBackground();
    else
      _texture = style.buttonUnpressFocusBackground();
    _forecolor = style.buttonFocusColor();
  } else {
    if (_pressed)
      _texture = style.buttonPressUnfocusBackground();
    else
      _texture = style.buttonUnpressUnfocusBackground();
    _forecolor = style.buttonUnfocusColor();
  }
  refresh();
}

}
