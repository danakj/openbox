// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __label_hh
#define __label_hh

#include "widget.hh"
#include "ustring.hh"
#include "renderstyle.hh"
#include "font.hh"

#include <vector>

namespace otk {

class Label : public Widget {

public:
  Label(Widget *parent);
  virtual ~Label();

  inline const ustring& text(void) const { return _text; }
  void setText(const ustring &text);

  virtual inline bool isHighlighted() const { return _highlight; }
  virtual void setHighlighted(bool h);
  
  RenderStyle::Justify horizontalJustify() const { return _justify_horz; }
  virtual void setHorizontalJustify(RenderStyle::Justify j);
  RenderStyle::Justify verticalJustify() const { return _justify_vert; }
  virtual void setVerticalJustify(RenderStyle::Justify j);

  const Font *font() const { return _font; }
  virtual void setFont(const Font *f);

  virtual void styleChanged(const RenderStyle &style);
  
  virtual void renderForeground(Surface &surface);

protected:
  virtual void calcDefaultSizes();

  //! The color the label will use for rendering its text
  RenderColor *_forecolor;
  
private:
  //! Text to be displayed in the label
  ustring _text;
  //! Text to be displayed, parsed into its separate lines
  std::vector<ustring> _parsedtext;
  //! The actual text being shown, may be a subset of _text
  ustring _drawtext;
  //! The font the text will be rendered with
  const Font *_font;
  //! The horizontal justification used for drawing text
  RenderStyle::Justify _justify_horz;
  //! The vertical justification used for drawing text
  RenderStyle::Justify _justify_vert;
  //! The drawing offset for the text
  int _drawx;
  //! If the widget is highlighted or not
  bool _highlight;
};

}

#endif
