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
}


LabelWidget::~LabelWidget()
{
}


void LabelWidget::setText(const otk::ustring &text)
{
  _text = text;
  _dirty = true;
}


void LabelWidget::setTextures()
{
  if (_focused) {
    setTexture(_style->labelFocusBackground());
    _text_color = _style->textFocusColor();
  } else {
    setTexture(_style->labelUnfocusBackground());
    _text_color = _style->textUnfocusColor();
  }
}


void LabelWidget::setStyle(otk::RenderStyle *style)
{
  otk::Widget::setStyle(style);
  setTextures();
  _font = style->labelFont();
  _sidemargin = style->bevelWidth() * 2;
  _justify = style->labelTextJustify();

  assert(_font);
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


void LabelWidget::renderForeground()
{
  bool draw = _dirty;

  otk::Widget::renderForeground();

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
      case otk::RenderStyle::RightJustify:
        x += max_length - length;
        break;
      case otk::RenderStyle::CenterJustify:
        x += (max_length - length) / 2;
        break;
      case otk::RenderStyle::LeftJustify:
        break;
      }
    }

    otk::display->renderControl(_screen)->drawString
      (*_surface, *_font, x, 0, *_text_color, t);
  }
}


void LabelWidget::adjust()
{
  // nothing to adjust. no children.
}

}
