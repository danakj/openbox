// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "label.hh"
#include "display.hh"
#include "rendercontrol.hh"

#include <string>

namespace otk {

Label::Label(Widget *parent)
  : Widget(parent),
    _text(""),
    _justify_horz(RenderStyle::LeftTopJustify),
    _justify_vert(RenderStyle::LeftTopJustify),
    _highlight(false)
{
  styleChanged(*RenderStyle::style(screen()));
}

Label::~Label()
{
}

void Label::setHorizontalJustify(RenderStyle::Justify j)
{
  _justify_horz = j;
  refresh();
}

void Label::setVerticalJustify(RenderStyle::Justify j)
{
  _justify_vert = j;
  refresh();
}

void Label::setHighlighted(bool h)
{
  _highlight = h;
  styleChanged(*RenderStyle::style(screen()));
  refresh();
}

void Label::setText(const ustring &text)
{
  bool utf = text.utf8();
  std::string s = text.c_str(); // use a normal string, for its functionality

  _parsedtext.clear();
  
  // parse it into multiple lines
  std::string::size_type p = 0;
  while (p != std::string::npos) {
    std::string::size_type p2 = s.find('\n', p);
    _parsedtext.push_back(s.substr(p, (p2==std::string::npos?p2:p2-p)));
    _parsedtext.back().setUtf8(utf);
    p = (p2==std::string::npos?p2:p2+1);
  }
  calcDefaultSizes();
}

void Label::setFont(const Font *f)
{
  _font = f;
  calcDefaultSizes();
}

void Label::calcDefaultSizes()
{
  int longest = 0;
  // find the longest line
  std::vector<ustring>::iterator it, end = _parsedtext.end();
  for (it = _parsedtext.begin(); it != end; ++it) {
    int length = _font->measureString(*it);
    if (length < 0) continue; // lines too long get skipped
    if (length > longest) longest = length;
  }
  setMinSize(Size(longest + borderWidth() * 2 + bevel() * 4,
                  _parsedtext.size() * _font->height() + borderWidth() * 2 +
                  bevel() * 2));
}
  
void Label::styleChanged(const RenderStyle &style)
{
  if (_highlight) {
    _texture = style.labelFocusBackground();
    _forecolor = style.textFocusColor();
  } else {
    _texture = style.labelUnfocusBackground();
    _forecolor = style.textUnfocusColor();
  }
  if (_font != style.labelFont()) {
    _font = style.labelFont();
    calcDefaultSizes();
  }
}

void Label::renderForeground(Surface &surface)
{
  const RenderControl *control = display->renderControl(screen());
  int sidemargin = bevel() * 2;
  int y = bevel();
  int w = area().width() - borderWidth() * 2 - sidemargin * 2;
  int h = area().height() - borderWidth() * 2 - bevel() * 2;

  switch (_justify_vert) {
  case RenderStyle::RightBottomJustify:
    y += h - (_parsedtext.size() * _font->height());
    if (y < bevel()) y = bevel();
    break;
  case RenderStyle::CenterJustify:
    y += (h - (_parsedtext.size() * _font->height())) / 2;
    if (y < bevel()) y = bevel();
    break;
  case RenderStyle::LeftTopJustify:
    break;
  }
  
  if (w <= 0) return; // can't fit anything
  
  std::vector<ustring>::iterator it, end = _parsedtext.end();
  for (it = _parsedtext.begin(); it != end; ++it, y += _font->height()) {
    ustring t = *it; // the actual text to draw
    int x = sidemargin;    // x coord for the text

    // find a string that will fit inside the area for text
    ustring::size_type text_len = t.size();
    int length;
      
    do {
      t.resize(text_len);
      length = _font->measureString(t);
    } while (length > w && text_len-- > 0);
    if (length < 0) continue; // lines too long get skipped

    if (text_len <= 0) continue; // won't fit anything

    // justify the text
    switch (_justify_horz) {
    case RenderStyle::RightBottomJustify:
      x += w - length;
      break;
    case RenderStyle::CenterJustify:
      x += (w - length) / 2;
      break;
    case RenderStyle::LeftTopJustify:
      break;
    }
 
    control->drawString(surface, *_font, x, y, *_forecolor, t);
 }
}

}
