#ifndef __label_hh
#define __label_hh

#include "widget.hh"

namespace otk {

class OtkLabel : public OtkWidget {

public:

  OtkLabel(OtkWidget *parent);
  ~OtkLabel();

  inline const std::string &getText(void) const { return _text; }
  void setText(const std::string &text) { _text = text; _dirty = true; }

  void update(void);

private:

  std::string _text;
};

}

#endif
