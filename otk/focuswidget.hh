#ifndef __focuswidget_hh
#define __focuswidget_hh

#include "widget.hh"
#include "application.hh"

namespace otk {

class OtkFocusWidget : public OtkWidget {

public:

  OtkFocusWidget(otk::OtkWidget *parent, Direction = Horizontal);
  virtual ~OtkFocusWidget();

  virtual void focus(void);
  virtual void unfocus(void);

  virtual void setTexture(otk::BTexture *texture);
  virtual void setBorderColor(const otk::BColor *color);

  inline void setUnfocusTexture(otk::BTexture *texture)
  { _unfocus_texture = texture; }
  inline otk::BTexture *getUnfocusTexture(void) const
  { return _unfocus_texture; }

  inline void setUnfocusBorderColor(const otk::BColor *color)
  { _unfocus_bcolor = color; }
  inline const otk::BColor *getUnfocusBorderColor(void) const
  { return _unfocus_bcolor; }

  inline bool isFocused(void) const { return _focused; }
  inline bool isUnfocused(void) const { return !_focused; }

private:

  BTexture *_unfocus_texture;
  BTexture *_focus_texture;

  const BColor *_unfocus_bcolor;
  const BColor *_focus_bcolor;
};

}

#endif // __focuswidget_hh
