// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __button_hh
#define __button_hh

#include "focuslabel.hh"

namespace otk {

class Button : public FocusLabel {

public:

  Button(Widget *parent);
  ~Button();

  inline const Texture *getPressedFocusTexture(void) const
  { return _pressed_focus_tx; }
  void setPressedFocusTexture(Texture *texture)
  { _pressed_focus_tx = texture; }

  inline const Texture *getPressedUnfocusTexture(void) const
  { return _pressed_unfocus_tx; }
  void setPressedUnfocusTexture(Texture *texture)
  { _pressed_unfocus_tx = texture; }

  void setTexture(Texture *texture);
  void setUnfocusTexture(Texture *texture);

  inline bool isPressed(void) const { return _pressed; }
  void press(unsigned int mouse_button);
  void release(unsigned int mouse_button);

  void buttonPressHandler(const XButtonEvent &e);
  void buttonReleaseHandler(const XButtonEvent &e);

  virtual void setStyle(Style *style);
  
private:

  bool _pressed;
  unsigned int _mouse_button;

  Texture *_pressed_focus_tx;
  Texture *_pressed_unfocus_tx;

  Texture *_unpr_focus_tx;
  Texture *_unpr_unfocus_tx;
};

}

#endif
