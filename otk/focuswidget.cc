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
  _focus_texture = parent->getTexture();
  _focus_bcolor = parent->getBorderColor();
}

OtkFocusWidget::~OtkFocusWidget()
{
}

#include <stdio.h>
void OtkFocusWidget::focus(void)
{
  if (!isVisible() || _focused)
    return;

  printf("FOCUS\n");
  OtkWidget::focus();

  if (_focus_bcolor)
    OtkWidget::setBorderColor(_focus_bcolor);

  OtkWidget::setTexture(_focus_texture);
  OtkWidget::update();
}

void OtkFocusWidget::unfocus(void)
{
  if (!isVisible() || !_focused)
    return;

  printf("UNFOCUS\n");
  OtkWidget::unfocus();

  if (_unfocus_bcolor)
    OtkWidget::setBorderColor(_unfocus_bcolor);

  OtkWidget::setTexture(_unfocus_texture);
  OtkWidget::update();

  OtkWidget::OtkWidgetList children = OtkWidget::getChildren();

  OtkWidget::OtkWidgetList::iterator it = children.begin(),
    end = children.end();

  OtkFocusWidget *tmp = 0;
  for (; it != end; ++it) {
    tmp = dynamic_cast<OtkFocusWidget*>(*it);
    if (tmp) tmp->unfocus();
  }
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
