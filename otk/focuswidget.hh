#ifndef __focuswidget_hh
#define __focuswidget_hh

#include "widget.hh"

namespace otk {

class OtkFocusWidget : public OtkWidget {

public:

  OtkFocusWidget(OtkWidget *parent, Direction = Horizontal);
  OtkFocusWidget(Style *style, Direction direction = Horizontal,
                 Cursor cursor = 0, int bevel_width = 1);

  virtual void focus(void);
  virtual void unfocus(void);

  void setTexture(BTexture *texture);

  inline void setUnfocusTexture(BTexture *texture)
  { _unfocus_texture = texture; }
  inline BTexture *getUnfocusTexture(void) const
  { return _unfocus_texture; }

  inline bool isFocused(void) const { return _focused; }
  inline bool isUnfocused(void) const { return !_focused; }

private:

  BTexture *_unfocus_texture;
  BTexture *_focus_texture;

  bool _focused;
};

}

#endif // __focuswidget_hh
