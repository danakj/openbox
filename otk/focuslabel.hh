#ifndef __label_hh
#define __label_hh

#include "focuswidget.hh"

namespace otk {

class OtkFocusLabel : public OtkFocusWidget {

public:

  OtkFocusLabel(OtkWidget *parent);
  ~OtkFocusLabel();

  inline const std::string &getText(void) const { return _text; }
  void setText(const std::string &text) { _text = text; _dirty = true; }

  void update(void);

private:

  std::string _text;
};

}

#endif
