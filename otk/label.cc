// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "label.hh"

namespace otk {

OtkLabel::OtkLabel(OtkWidget *parent)
  : OtkWidget(parent), _text("")
{
  const ScreenInfo *info = OBDisplay::screenInfo(getScreen());
  _xftdraw = XftDrawCreate(OBDisplay::display, getWindow(), info->getVisual(),
                           info->getColormap());
  
  setStyle(getStyle());
}

OtkLabel::~OtkLabel()
{
  XftDrawDestroy(_xftdraw);
}

void OtkLabel::setStyle(Style *style)
{
  OtkWidget::setStyle(style);

  setTexture(getStyle()->getLabelUnfocus());
}


void OtkLabel::update(void)
{
  if (_dirty) {
    const BFont &ft = getStyle()->getFont();
    unsigned int bevel = getStyle()->getBevelWidth() / 2;

    std::string t = _text; // the actual text to draw
    int x = bevel;         // x coord for the text

    // find a string that will fit inside the area for text
    int max_length = width() - bevel * 2;
    if (max_length <= 0) {
      t = ""; // can't fit anything
    } else {
      size_t text_len = t.size();
      int length;
      
      do {
        t.resize(text_len);
        length = ft.measureString(t);
      } while (length > max_length && text_len-- > 0);

      // justify the text
      switch (getStyle()->textJustify()) {
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

    OtkWidget::update();

    ft.drawString(_xftdraw, x, bevel, *getStyle()->getTextUnfocus(), t);
  } else
    OtkWidget::update();
}

}
