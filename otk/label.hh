#ifndef __label_hh
#define __label_hh

#include "widget.hh"
#include "font.hh"

namespace otk {

class OtkLabel : public OtkWidget {

public:

  OtkLabel(OtkWidget *parent);
  ~OtkLabel();

  inline const std::string &getText(void) const { return _text; }
  void setText(const std::string &text) { _text = text; _dirty = true; }

  void update(void);

private:
  //! Object used by Xft to render to the drawable
  XftDraw *_xftdraw;
  //! Text displayed in the label
  std::string _text;
};

}

#endif
