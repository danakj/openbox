// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "focuswidget.hh"

namespace otk {

OtkFocusWidget::OtkFocusWidget(OtkWidget *parent, Direction direction)
  : OtkWidget(parent, direction), _unfocus_texture(0), _unfocus_bcolor(0)
{
  _focused = true;
  _focus_texture = parent->texture();
  _focus_bcolor = parent->borderColor();
}

OtkFocusWidget::~OtkFocusWidget()
{
}

#include <stdio.h>
void OtkFocusWidget::focus(void)
{
  if (_focused)
    return;

  OtkWidget::focus();

  if (_focus_bcolor)
    OtkWidget::setBorderColor(_focus_bcolor);

  OtkWidget::setTexture(_focus_texture);
  update();
}

void OtkFocusWidget::unfocus(void)
{
  if (!_focused)
    return;

  OtkWidget::unfocus();

  if (_unfocus_bcolor)
    OtkWidget::setBorderColor(_unfocus_bcolor);

  OtkWidget::setTexture(_unfocus_texture);
  update();
}

void OtkFocusWidget::setTexture(BTexture *texture)
{
  OtkWidget::setTexture(texture);
  _focus_texture = texture;
}

void OtkFocusWidget::setBorderColor(const BColor *color)
{
  OtkWidget::setBorderColor(color);
  _focus_bcolor = color;
}

}
