// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "focuslabel.hh"
#include "display.hh"
#include "screeninfo.hh"

namespace otk {

FocusLabel::FocusLabel(Widget *parent)
  : FocusWidget(parent), _text("")
{
}

FocusLabel::~FocusLabel()
{
}


void FocusLabel::setStyle(RenderStyle *style)
{
  FocusWidget::setStyle(style);

  setTexture(style->labelFocusBackground());
  setUnfocusTexture(style->labelUnfocusBackground());
}


void FocusLabel::renderForeground()
{
  const Font *ft = style()->labelFont();
  RenderColor *text_color = (isFocused() ? style()->textFocusColor()
                             : style()->textUnfocusColor());
  unsigned int sidemargin = style()->bevelWidth() * 2;

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
    drawString(*_surface, *ft, x, 0, *text_color, t);
}

}
