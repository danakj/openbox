// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "widget.hh"

#include <algorithm>

namespace otk {

OtkWidget::OtkWidget(OtkWidget *parent, Direction direction)
  : OtkBaseWidget(parent),
    _direction(direction), _stretchable_vert(false), _stretchable_horz(false),
    _event_dispatcher(parent->getEventDispatcher())
{
  assert(parent);
  _event_dispatcher->registerHandler(getWindow(), this);
}

OtkWidget::OtkWidget(OtkEventDispatcher *event_dispatcher, Style *style,
                     Direction direction, Cursor cursor, int bevel_width)
  : OtkBaseWidget(style, cursor, bevel_width),
    _direction(direction), _stretchable_vert(false), _stretchable_horz(false),
    _event_dispatcher(event_dispatcher)
{
  assert(event_dispatcher);
  _event_dispatcher->registerHandler(getWindow(), this);
}

OtkWidget::~OtkWidget()
{
  _event_dispatcher->clearHandler(_window);
}

void OtkWidget::adjust(void)
{
  if (_direction == Horizontal)
    adjustHorz();
  else
    adjustVert();
}

void OtkWidget::adjustHorz(void)
{
  if (_children.size() == 0)
    return;

  OtkWidget *tmp;
  OtkBaseWidgetList::iterator it, end = _children.end();

  int tallest = 0;
  int width = _bevel_width;
  OtkBaseWidgetList stretchable;

  for (it = _children.begin(); it != end; ++it) {
    if (!(tmp = dynamic_cast<OtkWidget*>(*it))) continue;
    if (tmp->isStretchableVert())
      tmp->setHeight(_rect.height() > _bevel_width * 2 ?
                     _rect.height() - _bevel_width * 2 : _bevel_width);
    if (tmp->isStretchableHorz())
      stretchable.push_back(tmp);
    else
      width += tmp->_rect.width() + _bevel_width;

    if (tmp->_rect.height() > tallest)
      tallest = tmp->_rect.height();
  }

  if (stretchable.size() > 0) {
    OtkBaseWidgetList::iterator str_it = stretchable.begin(),
      str_end = stretchable.end();

    int str_width = _rect.width() - width / stretchable.size();

    for (; str_it != str_end; ++str_it)
      (*str_it)->setWidth(str_width > _bevel_width ? str_width - _bevel_width
                          : _bevel_width);
  }

  OtkWidget *prev_widget = 0;

  for (it = _children.begin(); it != end; ++it) {
    if (!(tmp = dynamic_cast<OtkWidget*>(*it))) continue;
    int x, y;

    if (prev_widget)
      x = prev_widget->_rect.x() + prev_widget->_rect.width() + _bevel_width;
    else
      x = _rect.x() + _bevel_width;
    y = (tallest - tmp->_rect.height()) / 2 + _bevel_width;

    tmp->move(x, y);

    prev_widget = tmp;
  }

  internalResize(width, tallest + _bevel_width * 2);
}

void OtkWidget::adjustVert(void)
{
  if (_children.size() == 0)
    return;

  OtkWidget *tmp;
  OtkBaseWidgetList::iterator it, end = _children.end();

  int widest = 0;
  int height = _bevel_width;
  OtkBaseWidgetList stretchable;

  for (it = _children.begin(); it != end; ++it) {
    if (!(tmp = dynamic_cast<OtkWidget*>(*it))) continue;
    if (tmp->isStretchableHorz())
      tmp->setWidth(_rect.width() > _bevel_width * 2 ?
                    _rect.width() - _bevel_width * 2 : _bevel_width);
    if (tmp->isStretchableVert())
      stretchable.push_back(tmp);
    else
      height += tmp->_rect.height() + _bevel_width;

    if (tmp->_rect.width() > widest)
      widest = tmp->_rect.width();
  }

  if (stretchable.size() > 0) {
    OtkBaseWidgetList::iterator str_it = stretchable.begin(),
      str_end = stretchable.end();

    int str_height = _rect.height() - height / stretchable.size();

    for (; str_it != str_end; ++str_it)
      (*str_it)->setHeight(str_height > _bevel_width ?
                           str_height - _bevel_width : _bevel_width);
  }

  OtkWidget *prev_widget = 0;

  for (it = _children.begin(); it != end; ++it) {
    if (!(tmp = dynamic_cast<OtkWidget*>(*it))) continue;
    int x, y;

    if (prev_widget)
      y = prev_widget->_rect.y() + prev_widget->_rect.height() + _bevel_width;
    else
      y = _rect.y() + _bevel_width;
    x = (widest - tmp->_rect.width()) / 2 + _bevel_width;

    tmp->move(x, y);

    prev_widget = tmp;
  }

  internalResize(widest + _bevel_width * 2, height);
}

void OtkWidget::update(void)
{
  if (_dirty)
    adjust();

  OtkBaseWidget::update();
}

void OtkWidget::internalResize(int w, int h)
{
  assert(w > 0 && h > 0);

//  if (! _fixed_width && ! _fixed_height)
    resize(w, h);
//  else if (! _fixed_width)
//    resize(w, _rect.height());
//  else if (! _fixed_height)
//    resize(_rect.width(), h);
}

void OtkWidget::setEventDispatcher(OtkEventDispatcher *disp)
{
  if (_event_dispatcher)
    _event_dispatcher->clearHandler(_window);
  _event_dispatcher = disp;
  _event_dispatcher->registerHandler(_window, this);
}


}
