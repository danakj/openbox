// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __focuslabel_hh
#define __focuslabel_hh

#include "focuswidget.hh"

namespace otk {

class FocusLabel : public FocusWidget {

public:

  FocusLabel(Widget *parent);
  ~FocusLabel();

  inline const ustring &getText(void) const { return _text; }
  void setText(const ustring &text) { _text = text; _dirty = true; }

  virtual void renderForeground();

  virtual void setStyle(RenderStyle *style);
  
private:
  //! Text displayed in the label
  ustring _text;
};

}

#endif // __focuslabel_hh
