// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __label_hh
#define __label_hh

#include "widget.hh"
#include "font.hh"

namespace otk {

class Label : public Widget {

public:

  Label(Widget *parent);
  ~Label();

  inline const std::string &getText(void) const { return _text; }
  void setText(const std::string &text) { _text = text; _dirty = true; }

  void update(void);

  virtual void setStyle(Style *style);

private:
  //! Object used by Xft to render to the drawable
  XftDraw *_xftdraw;
  //! Text displayed in the label
  std::string _text;
};

}

#endif
