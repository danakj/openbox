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

  void update(void);

  virtual void setStyle(Style *style);

private:
  //! Text displayed in the label
  ustring _text;
};

}

#endif
