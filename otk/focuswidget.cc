// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "focuswidget.hh"

namespace otk {

OtkFocusWidget::OtkFocusWidget(OtkWidget *parent, Direction direction)
  : OtkWidget(parent, direction), _unfocus_texture(0), _focused(true)
{
  _focus_texture = parent->getTexture();
}

OtkFocusWidget::~OtkFocusWidget()
{
}

void OtkFocusWidget::focus(void)
{
  if (_focused)
    return;

  // XXX: what about OtkWidget::focus()

  assert(_focus_texture);
  OtkWidget::setTexture(_focus_texture);
  OtkWidget::update();

  OtkBaseWidgetList::iterator it = _children.begin(), end = _children.end();
  OtkFocusWidget *tmp = 0;
  for (; it != end; ++it) {
    tmp = dynamic_cast<OtkFocusWidget*>(*it);
    if (tmp) tmp->focus();
  }
}

void OtkFocusWidget::unfocus(void)
{
  if (! _focused)
    return;

  assert(_unfocus_texture);
  OtkWidget::setTexture(_unfocus_texture);
  OtkWidget::update();

  OtkBaseWidgetList::iterator it = _children.begin(), end = _children.end();
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

}
