// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __label_hh
#define __label_hh

#include "widget.hh"

namespace otk {

class Label : public Widget {

public:

  Label(Widget *parent);
  ~Label();

  inline const ustring &getText(void) const { return _text; }
  void setText(const ustring &text) { _text = text; _dirty = true; }

  virtual void renderForeground(void);

  virtual void update();

  void fitString(const std::string &str);
  void fitSize(int w, int h);

  virtual void setStyle(RenderStyle *style);

private:
  //! Text to be displayed in the label
  ustring _text;
  //! The actual text being shown, may be a subset of _text
  ustring _drawtext;
  //! The drawing offset for the text
  int _drawx;
};

}

#endif
