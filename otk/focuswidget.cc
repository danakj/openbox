#include "focuswidget.hh"

namespace otk {

OtkFocusWidget::OtkFocusWidget(OtkWidget *parent, Direction direction)
  : OtkWidget(parent, direction), _unfocus_texture(0), _focused(true)
{
  _focus_texture = parent->getTexture();
}

OtkFocusWidget::OtkFocusWidget(Style *style, Direction direction,
                               Cursor cursor, int bevel_width)
  : OtkWidget(style, direction, cursor, bevel_width),
    _unfocus_texture(0), _focused(true)
{
}

void OtkFocusWidget::focus(void)
{
  if (_focused)
    return;

  assert(_focus_texture);
  OtkWidget::setTexture(_focus_texture);
  OtkWidget::update();

  OtkWidget::OtkWidgetList children = OtkWidget::getChildren();

  OtkWidget::OtkWidgetList::iterator it = children.begin(),
    end = children.end();

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

}
