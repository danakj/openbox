// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "focuswidget.hh"

namespace otk {

FocusWidget::FocusWidget(Widget *parent, Direction direction)
  : Widget(parent, direction), _unfocus_texture(0), _unfocus_bcolor(0)
{
  _focused = true;
  _focus_texture = parent->texture();
  _focus_bcolor = parent->borderColor();
}

FocusWidget::~FocusWidget()
{
}


void FocusWidget::focus(void)
{
  if (_focused)
    return;

  Widget::focus();

  if (_focus_bcolor)
    Widget::setBorderColor(_focus_bcolor);

  Widget::setTexture(_focus_texture);
  update();
}

void FocusWidget::unfocus(void)
{
  if (!_focused)
    return;

  Widget::unfocus();

  if (_unfocus_bcolor)
    Widget::setBorderColor(_unfocus_bcolor);

  Widget::setTexture(_unfocus_texture);
  update();
}

void FocusWidget::setTexture(RenderTexture *texture)
{
  Widget::setTexture(texture);
  _focus_texture = texture;
  if (!_focused)
    Widget::setTexture(_unfocus_texture);
}

void FocusWidget::setBorderColor(const RenderColor *color)
{
  Widget::setBorderColor(color);
  _focus_bcolor = color;
}

}
