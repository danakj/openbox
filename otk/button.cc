#include "button.hh"

namespace otk {

OtkButton::OtkButton(OtkWidget *parent)
  : OtkFocusLabel(parent), _pressed(false), _pressed_focus_tx(0),
    _pressed_unfocus_tx(0), _unpr_focus_tx(0), _unpr_unfocus_tx(0)
{
  setTexture(getStyle()->getButtonFocus());
  setUnfocusTexture(getStyle()->getButtonUnfocus());
  _pressed_focus_tx = getStyle()->getButtonPressedFocus();
  _pressed_unfocus_tx = getStyle()->getButtonPressedUnfocus();
}

OtkButton::~OtkButton()
{
  if (_pressed_focus_tx) delete _pressed_focus_tx;
  if (_pressed_unfocus_tx) delete _pressed_unfocus_tx;
}

void OtkButton::press(unsigned int mouse_button)
{
  if (_pressed) return;

  if (_pressed_focus_tx)
    OtkFocusWidget::setTexture(_pressed_focus_tx);
  if (_pressed_unfocus_tx)
    OtkFocusWidget::setUnfocusTexture(_pressed_unfocus_tx);
  _pressed = true;
  _mouse_button = mouse_button;
}

void OtkButton::release(unsigned int mouse_button)
{
  if (_mouse_button != mouse_button) return; // wrong button

  OtkFocusWidget::setTexture(_unpr_focus_tx);
  OtkFocusWidget::setUnfocusTexture(_unpr_unfocus_tx);
  _pressed = false;
}

void OtkButton::setTexture(BTexture *texture)
{
  OtkFocusWidget::setTexture(texture);
  _unpr_focus_tx = texture;
}

void OtkButton::setUnfocusTexture(BTexture *texture)
{
  OtkFocusWidget::setUnfocusTexture(texture);
  _unpr_unfocus_tx = texture;
}

void OtkButton::buttonPressHandler(const XButtonEvent &e)
{
  press(e.button);
  update();
  OtkFocusWidget::buttonPressHandler(e);
}

void OtkButton::buttonReleaseHandler(const XButtonEvent &e)
{
  release(e.button);
  update();
  OtkFocusWidget::buttonReleaseHandler(e);
}

}
