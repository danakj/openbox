// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __messagedialog_hh
#define __messagedialog_hh

#include "widget.hh"
#include "ustring.hh"

#include <vector>

namespace otk {

class Button;
class Label;

class DialogButton {
  ustring _label;
  bool _default;
public:
  DialogButton(char *label) : _label(label), _default(false)
    {}
  DialogButton(char *label, bool def) : _label(label), _default(def)
    {}
  inline const ustring& label() const { return _label; }
  inline const bool& isDefault() const { return _default; }
};

class MessageDialog : public Widget {
public:
  MessageDialog(int screen, EventDispatcher *ed, ustring title,
		ustring caption);
  MessageDialog(EventDispatcher *ed, ustring title, ustring caption);
  MessageDialog(Widget *parent, ustring title, ustring caption);
  virtual ~MessageDialog();

  virtual void addButton(const DialogButton &b) { _buttons.push_back(b); }

  virtual const DialogButton& run();

  virtual void show();
  virtual void hide();
  virtual void focus();

  virtual const DialogButton& result() const { return *_result; }
  virtual void setResult(const DialogButton &result) { _result = &result; }
  
  virtual void keyPressHandler(const XKeyEvent &e);
  virtual void clientMessageHandler(const XClientMessageEvent &e);

private:
  static DialogButton _default_result;

  void init(const ustring &title, const ustring &caption);

  std::vector<DialogButton> _buttons;
  std::vector<Button *> _button_widgets;
  Label *_label;
  Widget *_button_holder;
  KeyCode _return;
  KeyCode _escape;
  const DialogButton *_result;
};

}

#endif // __messagedialog_hh
