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


void FocusLabel::setStyle(Style *style)
{
  FocusWidget::setStyle(style);

  // XXX: do this again
  //setTexture(style->getLabelFocus());
  //setUnfocusTexture(style->getLabelUnfocus());
}


void FocusLabel::renderForeground(void)
{
  const Font *ft = style()->getFont();
  Color *text_color = (isFocused() ? style()->getTextFocus()
                       : style()->getTextUnfocus());
  unsigned int sidemargin = style()->getBevelWidth() * 2;

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
    switch (style()->textJustify()) {
    case Style::RightJustify:
      x += max_length - length;
      break;
    case Style::CenterJustify:
      x += (max_length - length) / 2;
      break;
    case Style::LeftJustify:
      break;
    }
  }

  display->renderControl(_screen)->
    drawString(_surface, *ft, x, 0, *text_color, t);
}

}
