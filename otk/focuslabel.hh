// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __label_hh
#define __label_hh

#include "focuswidget.hh"

namespace otk {

class FocusLabel : public FocusWidget {

public:

  FocusLabel(Widget *parent);
  ~FocusLabel();

  inline const ustring &getText(void) const { return _text; }
  void setText(const ustring &text) { _text = text; _dirty = true; }

  void renderForeground(void);

  virtual void setStyle(RenderStyle *style);
  
private:
  //! Text displayed in the label
  ustring _text;
};

}

#endif
