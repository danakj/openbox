#ifndef __button_hh
#define __button_hh

#include "focuswidget.hh"
//#include "pixmap.hh"

namespace otk {

class OtkButton : public OtkFocusWidget {

public:

  OtkButton(OtkWidget *parent);
  ~OtkButton();

  inline const BTexture *getPressedFocusTexture(void) const
  { return _pressed_focus_tx; }
  void setPressedFocusTexture(BTexture *texture)
  { _pressed_focus_tx = texture; }

  inline const BTexture *getPressedUnfocusTexture(void) const
  { return _pressed_unfocus_tx; }
  void setPressedUnfocusTexture(BTexture *texture)
  { _pressed_unfocus_tx = texture; }

  void setTexture(BTexture *texture);
  void setUnfocusTexture(BTexture *texture);

  inline const std::string &getText(void) const { return _text; }
  void setText(const std::string &text) { _text = text; _dirty = true; }

  //inline const OtkPixmap &getPixmap(void) const { return _pixmap; }
  //void setPixmap(const OtkPixmap &pixmap);

  inline bool isPressed(void) const { return _pressed; }
  void press(void);
  void release(void);

  void update(void);
  int exposeHandler(const XExposeEvent &e);
  int configureHandler(const XConfigureEvent &e);
  int buttonPressHandler(const XButtonEvent &e);
  int buttonReleaseHandler(const XButtonEvent &e);

private:

  std::string _text;
  //OtkPixmap _pixmap;
  bool _pressed;
  bool _dirty;

  BTexture *_pressed_focus_tx;
  BTexture *_pressed_unfocus_tx;

  BTexture *_unpr_focus_tx;
  BTexture *_unpr_unfocus_tx;
};

}

#endif
