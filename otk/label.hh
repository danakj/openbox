// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __label_hh
#define __label_hh

#include "widget.hh"
#include "font.hh"
#include "userstring.hh"

namespace otk {

class Label : public Widget {

public:

  Label(Widget *parent);
  ~Label();

  inline const userstring &getText(void) const { return _text; }
  void setText(const userstring &text) { _text = text; _dirty = true; }

  void update(void);

  virtual void setStyle(Style *style);

private:
  //! Object used by Xft to render to the drawable
  XftDraw *_xftdraw;
  //! Text displayed in the label
  userstring _text;
};

}

#endif
