#ifndef __button_hh
#define __button_hh

#include "focuslabel.hh"

namespace otk {

class OtkButton : public OtkFocusLabel {

public:

  OtkButton(OtkWidget *parent);
  ~OtkButton();

  inline const otk::BTexture *getPressedFocusTexture(void) const
  { return _pressed_focus_tx; }
  void setPressedFocusTexture(otk::BTexture *texture)
  { _pressed_focus_tx = texture; }

  inline const otk::BTexture *getPressedUnfocusTexture(void) const
  { return _pressed_unfocus_tx; }
  void setPressedUnfocusTexture(otk::BTexture *texture)
  { _pressed_unfocus_tx = texture; }

  void setTexture(otk::BTexture *texture);
  void setUnfocusTexture(otk::BTexture *texture);

  inline bool isPressed(void) const { return _pressed; }
  void press(unsigned int mouse_button);
  void release(unsigned int mouse_button);

  void buttonPressHandler(const XButtonEvent &e);
  void buttonReleaseHandler(const XButtonEvent &e);

  virtual void setStyle(otk::Style *style);
  
private:

  bool _pressed;
  unsigned int _mouse_button;

  BTexture *_pressed_focus_tx;
  BTexture *_pressed_unfocus_tx;

  BTexture *_unpr_focus_tx;
  BTexture *_unpr_unfocus_tx;
};

}

#endif
