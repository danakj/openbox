// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "label.hh"

namespace otk {

Label::Label(Widget *parent)
  : Widget(parent), _text("")
{
}

Label::~Label()
{
}

void Label::setStyle(Style *style)
{
  Widget::setStyle(style);

  // XXX: do this again
  //setTexture(style->getLabelUnfocus());
}


void Label::renderForeground(void)
{
  const Font *ft = style()->getFont();
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
    drawString(*_surface, *ft, x, 0, *style()->getTextUnfocus(), t);
}

}
