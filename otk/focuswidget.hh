// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __focuswidget_hh
#define __focuswidget_hh

#include "widget.hh"
#include "application.hh"

namespace otk {

class FocusWidget : public Widget {

public:

  FocusWidget(Widget *parent, Direction = Horizontal);
  virtual ~FocusWidget();

  virtual void focus(void);
  virtual void unfocus(void);

  virtual void setTexture(Texture *texture);
  virtual void setBorderColor(const Color *color);

  inline void setUnfocusTexture(Texture *texture)
  { _unfocus_texture = texture; }
  inline Texture *getUnfocusTexture(void) const
  { return _unfocus_texture; }

  inline void setUnfocusBorderColor(const Color *color)
  { _unfocus_bcolor = color; }
  inline const Color *getUnfocusBorderColor(void) const
  { return _unfocus_bcolor; }

  inline bool isFocused(void) const { return _focused; }
  inline bool isUnfocused(void) const { return !_focused; }

private:

  Texture *_unfocus_texture;
  Texture *_focus_texture;

  const Color *_unfocus_bcolor;
  const Color *_focus_bcolor;
};

}

#endif // __focuswidget_hh
