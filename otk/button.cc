#include "button.hh"

namespace otk {

OtkButton::OtkButton(OtkWidget *parent)
  : OtkFocusWidget(parent), _text(""), _pressed(false), _dirty(false),
    _pressed_focus_tx(0), _pressed_unfocus_tx(0), _unpr_focus_tx(0),
    _unpr_unfocus_tx(0)
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

void OtkButton::press(void)
{
  if (_pressed_focus_tx)
    OtkFocusWidget::setTexture(_pressed_focus_tx);
  if (_pressed_unfocus_tx)
    OtkFocusWidget::setUnfocusTexture(_pressed_unfocus_tx);
  _pressed = true;
}

void OtkButton::release(void)
{
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

void OtkButton::update(void)
{
  if (_dirty) {
    const BFont ft = getStyle()->getFont();
    BColor *text_color = (isFocused() ? getStyle()->getTextFocus()
                          : getStyle()->getTextUnfocus());
    unsigned int bevel = getStyle()->getBevelWidth();

    OtkFocusWidget::resize(ft.measureString(_text) + bevel * 2,
                           ft.height() + bevel * 2);

    OtkFocusWidget::update();

    ft.drawString(getWindow(), bevel, bevel, *text_color, _text);
  } else
    OtkFocusWidget::update();
}

void OtkButton::expose(const XExposeEvent &e)
{
  _dirty = true;
  OtkFocusWidget::expose(e);
}

}
