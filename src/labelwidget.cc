// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "otk/screeninfo.hh"
#include "otk/display.hh"
#include "labelwidget.hh"

namespace ob {

LabelWidget::LabelWidget(otk::Widget *parent, WidgetBase::WidgetType type)
  : otk::Widget(parent),
    WidgetBase(type)
{
  const otk::ScreenInfo *info = otk::Display::screenInfo(_screen);
  _xftdraw = XftDrawCreate(otk::Display::display, _window, info->visual(),
                           info->colormap());
}


LabelWidget::~LabelWidget()
{
  XftDrawDestroy(_xftdraw);
}


void LabelWidget::setText(const otk::ustring &text)
{
  _text = text;
  _dirty = true;
}


void LabelWidget::setTextures()
{
  if (_focused) {
    setTexture(_style->getLabelFocus());
    _text_color = _style->getTextFocus();
  } else {
    setTexture(_style->getLabelUnfocus());
    _text_color = _style->getTextUnfocus();
  }
}


void LabelWidget::setStyle(otk::Style *style)
{
  otk::Widget::setStyle(style);
  setTextures();
  _font = style->getFont();
  assert(_font);
  _sidemargin = style->getBevelWidth() * 2;
  _justify = style->textJustify();
}


void LabelWidget::focus()
{
  otk::Widget::focus();
  setTextures();
}


void LabelWidget::unfocus()
{
  otk::Widget::unfocus();
  setTextures();
}


void LabelWidget::update()
{
  bool draw = _dirty;

  otk::Widget::update();

  if (draw) {
    otk::ustring t = _text;
    int x = _sidemargin;    // x coord for the text

    // find a string that will fit inside the area for text
    int max_length = width() - _sidemargin * 2;
    if (max_length <= 0) {
      t = ""; // can't fit anything
    } else {
      size_t text_len = t.size();
      int length;
      
      do {
        t.resize(text_len);
        length = _font->measureString(t);
      } while (length > max_length && text_len-- > 0);

      // justify the text
      switch (_justify) {
      case otk::Style::RightJustify:
        x += max_length - length;
        break;
      case otk::Style::CenterJustify:
        x += (max_length - length) / 2;
        break;
      case otk::Style::LeftJustify:
        break;
      }
    }

    _font->drawString(_xftdraw, x, 0, *_text_color, t);
  }
}


void LabelWidget::adjust()
{
  // nothing to adjust. no children.
}

}
