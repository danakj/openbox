// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "label.hh"

namespace otk {

Label::Label(Widget *parent)
  : Widget(parent), _text("")
{
  setStyle(_style);
}

Label::~Label()
{
}

void Label::setStyle(RenderStyle *style)
{
  Widget::setStyle(style);

  setTexture(style->labelUnfocusBackground());
}

void Label::fitString(const std::string &str)
{
  const Font *ft = style()->labelFont();
  fitSize(ft->measureString(str), ft->height());
}

void Label::fitSize(int w, int h)
{
  unsigned int sidemargin = _bevel_width * 2;
  resize(w + sidemargin * 2, h + _bevel_width * 2);
}

void Label::update()
{
  if (_dirty) {
    int w = _rect.width(), h = _rect.height();
    const Font *ft = style()->labelFont();
    unsigned int sidemargin = _bevel_width * 2;
    if (!_fixed_width)
      w = ft->measureString(_text) + sidemargin * 2;
    if (!_fixed_height)
      h = ft->height();

    // enforce a minimum size
    if (w > _rect.width()) {
      if (h > _rect.height())
        internalResize(w, h);
      else
        internalResize(w, _rect.height());
    } else if (h > _rect.height())
      internalResize(_rect.width(), h);
  }
  Widget::update();
}


void Label::renderForeground(void)
{
  Widget::renderForeground();

  const Font *ft = style()->labelFont();
  unsigned int sidemargin = _bevel_width * 2;

  ustring t = _text; // the actual text to draw
  int x = sidemargin;    // x coord for the text

  // find a string that will fit inside the area for text
  int max_length = width() - sidemargin * 2;
  if (max_length <= 0) {
    t = ""; // can't fit anything
  } else {
    size_t text_len = t.size();
    int length;
      
    do {
      t.resize(text_len);
      length = ft->measureString(t);
    } while (length > max_length && text_len-- > 0);

    // justify the text
    switch (style()->labelTextJustify()) {
    case RenderStyle::RightJustify:
      x += max_length - length;
      break;
    case RenderStyle::CenterJustify:
      x += (max_length - length) / 2;
      break;
    case RenderStyle::LeftJustify:
      break;
    }
  }

  display->renderControl(_screen)->
    drawString(*_surface, *ft, x, _bevel_width, *style()->textUnfocusColor(), t);
}

}
