#include "label.hh"

namespace otk {

OtkLabel::OtkLabel(OtkWidget *parent)
  : OtkWidget(parent), _text(""), _dirty(false)
{
  setTexture(getStyle()->getLabelUnfocus());
}

OtkLabel::~OtkLabel()
{
}

void OtkLabel::update(void)
{
  if (_dirty) {
    const BFont &ft = getStyle()->getFont();
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

    OtkWidget::update();

    ft.drawString(getWindow(), x, bevel, *getStyle()->getTextUnfocus(), t);
  } else
    OtkWidget::update();

  _dirty = false;
}

int OtkLabel::exposeHandler(const XExposeEvent &e)
{
  _dirty = true;
  return OtkWidget::exposeHandler(e);
}

int OtkLabel::configureHandler(const XConfigureEvent &e)
{
  if (!(e.width == width() && e.height == height()))
    _dirty = true;
  return OtkWidget::configureHandler(e);
}

}
