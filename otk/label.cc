// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "label.hh"

namespace otk {

Label::Label(Widget *parent)
  : Widget(parent), _text("")
{
  const ScreenInfo *info = Display::screenInfo(screen());
  _xftdraw = XftDrawCreate(Display::display, window(), info->visual(),
                           info->colormap());
}

Label::~Label()
{
  XftDrawDestroy(_xftdraw);
}

void Label::setStyle(Style *style)
{
  Widget::setStyle(style);

  setTexture(style->getLabelUnfocus());
}


void Label::update(void)
{
  if (_dirty) {
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

    Widget::update();

    ft->drawString(_xftdraw, x, 0, *style()->getTextUnfocus(), t);
  } else
    Widget::update();
}

}
