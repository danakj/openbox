// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "otk/screeninfo.hh"
#include "otk/display.hh"
#include "labelwidget.hh"

namespace ob {

OBLabelWidget::OBLabelWidget(otk::OtkWidget *parent, OBWidget::WidgetType type)
  : otk::OtkWidget(parent),
    OBWidget(type)
{
  const otk::ScreenInfo *info = otk::OBDisplay::screenInfo(_screen);
  _xftdraw = XftDrawCreate(otk::OBDisplay::display, _window, info->visual(),
                           info->colormap());
}


OBLabelWidget::~OBLabelWidget()
{
  XftDrawDestroy(_xftdraw);
}


void OBLabelWidget::setText(const std::string &text)
{
  _text = text;
  _dirty = true;
}


void OBLabelWidget::setTextures()
{
  if (_focused) {
    setTexture(_style->getLabelFocus());
    _text_color = _style->getTextFocus();
  } else {
    setTexture(_style->getLabelUnfocus());
    _text_color = _style->getTextUnfocus();
  }
}


void OBLabelWidget::setStyle(otk::Style *style)
{
  OtkWidget::setStyle(style);
  setTextures();
  _font = style->getFont();
  assert(_font);
  _sidemargin = style->getBevelWidth() * 2;
  _justify = style->textJustify();
}


void OBLabelWidget::focus()
{
  otk::OtkWidget::focus();
  setTextures();
}


void OBLabelWidget::unfocus()
{
  otk::OtkWidget::unfocus();
  setTextures();
}


void OBLabelWidget::update()
{
  if (_dirty) {
    std::string t = _text;
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

    OtkWidget::update();

    _font->drawString(_xftdraw, x, 0, *_text_color, t);
  } else
    OtkWidget::update();
}


void OBLabelWidget::adjust()
{
  // XXX: adjust shit
}

}
