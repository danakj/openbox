// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __button_hh
#define __button_hh

#include "focuslabel.hh"

namespace otk {

class Button : public FocusLabel {

public:

  Button(Widget *parent);
  ~Button();

  inline const RenderTexture *getPressedFocusTexture(void) const
  { return _pressed_focus_tx; }
  void setPressedFocusTexture(RenderTexture *texture)
  { _pressed_focus_tx = texture; }

  inline const RenderTexture *getPressedUnfocusTexture(void) const
  { return _pressed_unfocus_tx; }
  void setPressedUnfocusTexture(RenderTexture *texture)
  { _pressed_unfocus_tx = texture; }

  void setTexture(RenderTexture *texture);
  void setUnfocusTexture(RenderTexture *texture);

  inline bool isPressed(void) const { return _pressed; }
  void press(unsigned int mouse_button);
  void release(unsigned int mouse_button);

  void buttonPressHandler(const XButtonEvent &e);
  void buttonReleaseHandler(const XButtonEvent &e);

  virtual void setStyle(Style *style);
  
private:

  bool _pressed;
  unsigned int _mouse_button;

  RenderTexture *_pressed_focus_tx;
  RenderTexture *_pressed_unfocus_tx;

  RenderTexture *_unpr_focus_tx;
  RenderTexture *_unpr_unfocus_tx;
};

}

#endif
