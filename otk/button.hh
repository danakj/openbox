// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __button_hh
#define __button_hh

#include "label.hh"

namespace otk {

class Button : public Label {

public:
  Button(Widget *parent);
  virtual ~Button();

  virtual inline bool isPressed() const { return _pressed; }

  virtual void press(unsigned int mouse_button);
  virtual void release(unsigned int mouse_button);

  virtual void buttonPressHandler(const XButtonEvent &e);
  virtual void buttonReleaseHandler(const XButtonEvent &e);

  virtual void styleChanged(const RenderStyle &style);
 
private:
  bool _pressed;
  unsigned int _mouse_button;
};

}

#endif
