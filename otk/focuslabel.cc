#include "focuslabel.hh"

namespace otk {

OtkFocusLabel::OtkFocusLabel(OtkWidget *parent)
  : OtkFocusWidget(parent), _text("")
{
  setTexture(getStyle()->getLabelFocus());
  setUnfocusTexture(getStyle()->getLabelUnfocus());
}

OtkFocusLabel::~OtkFocusLabel()
{
}

void OtkFocusLabel::update(void)
{
  if (_dirty) {
    const BFont &ft = getStyle()->getFont();
    BColor *text_color = (isFocused() ? getStyle()->getTextFocus()
                          : getStyle()->getTextUnfocus());
    unsigned int bevel = getStyle()->getBevelWidth();

    std::string t = _text; // the actual text to draw
    int x = bevel;         // x coord for the text

    // find a string that will fit inside the area for text
    int max_length = width() - getBevelWidth() * 2;
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

    OtkFocusWidget::update();

    ft.drawString(getWindow(), x, bevel, *text_color, t);
  } else
    OtkFocusWidget::update();
}

}
