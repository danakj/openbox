// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "messagedialog.hh"
#include "assassin.hh"
#include "button.hh"
#include "label.hh"
#include "display.hh"
#include "property.hh"
#include "eventdispatcher.hh"
#include "timer.hh"

#include <algorithm>

namespace otk {

DialogButton MessageDialog::_default_result("", false);

class DialogButtonWidget : public Button {
  MessageDialog *_dia;
  const DialogButton &_res;
public:
  DialogButtonWidget(Widget *parent, MessageDialog *dia,
		     const DialogButton &b)
    : Button(parent),
      _dia(dia),
      _res(b)
  {
    assert(dia);
    setBevel(1);
    setMaxSize(Size(0,0));
    setText(b.label());
    setHighlighted(b.isDefault());
    show();
  }

  virtual void buttonPressHandler(const XButtonEvent &e) {
    // limit to the left button
    if (e.button == Button1)
      Button::buttonPressHandler(e);
  }
  virtual void clickHandler(unsigned int) {
    _dia->setResult(_res);
    _dia->hide();
  }
};

MessageDialog::MessageDialog(int screen, EventDispatcher *ed, ustring title,
			     ustring caption)
  : Widget(screen, ed, Widget::Vertical)
{
  init(title, caption);
}

MessageDialog::MessageDialog(EventDispatcher *ed, ustring title,
			     ustring caption)
  : Widget(DefaultScreen(**display), ed, Widget::Vertical)
{
  init(title, caption);
}

MessageDialog::MessageDialog(Widget *parent, ustring title, ustring caption)
  : Widget(parent, Widget::Vertical)
{
  init(title, caption);
}

void MessageDialog::init(const ustring &title, const ustring &caption)
{
  _label = new Label(this);
  _label->show();
  _label->setHighlighted(true);
  _button_holder = new Widget(this, Widget::Horizontal);
  _button_holder->show();
  _return = XKeysymToKeycode(**display, XStringToKeysym("Return"));
  _escape = XKeysymToKeycode(**display, XStringToKeysym("Escape"));
  _result = &_default_result;

  setEventMask(eventMask() | KeyPressMask);
  _label->setText(caption);
  if (title.utf8())
    otk::Property::set(window(), otk::Property::atoms.net_wm_name,
		       otk::Property::utf8, title);
  otk::Property::set(window(), otk::Property::atoms.wm_name,
		     otk::Property::ascii, otk::ustring(title.c_str(), false));

  // set WM Protocols on the window
  Atom protocols[2];
  protocols[0] = Property::atoms.wm_protocols;
  protocols[1] = Property::atoms.wm_delete_window;
  XSetWMProtocols(**display, window(), protocols, 2);
}

MessageDialog::~MessageDialog()
{
  if (visible()) hide();
  delete _button_holder;
  delete _label;
}

const DialogButton& MessageDialog::run()
{
  if (!visible())
    show();

  while (visible()) {
    dispatcher()->dispatchEvents();
    if (visible())
      Timer::dispatchTimers(); // fire pending events
  }
  return *_result;
}

void MessageDialog::focus()
{
  if (visible())
    XSetInputFocus(**display, window(), None, CurrentTime);
}

void MessageDialog::show()
{
  std::vector<DialogButton>::const_iterator it, end = _buttons.end();
  for (it = _buttons.begin(); it != end; ++it)
    _button_widgets.push_back(new DialogButtonWidget(_button_holder,
						     this, *it));

  Rect r;

  if (parent())
    r = parent()->area();
  else
    r = Rect(Point(0, 0), display->screenInfo(screen())->size());
  
  XSizeHints size;
  size.flags = PMinSize | PPosition | PWinGravity;
  size.min_width = minSize().width();
  size.min_height = minSize().height();
  size.win_gravity = CenterGravity;

  Size dest = minSize();
  if (dest.width() < 200 || dest.height() < 100) {
    if (dest.width() < 200 && dest.height() < 100) dest = Size(200, 100);
    else if (dest.width() < 200) dest = Size(200, dest.height());
    else dest = Size(dest.width(), 100);
    resize(dest);
  }

  // center it above its parent
  move(Point(r.x() + (r.width() - dest.width()) / 2,
	     r.y() + (r.height() - dest.height()) / 2));
  
  XSetWMNormalHints(**display, window(), &size);
  
  Widget::show();
}

void MessageDialog::hide()
{
  Widget::hide();
  std::for_each(_button_widgets.begin(), _button_widgets.end(),
		PointerAssassin());
}

void MessageDialog::keyPressHandler(const XKeyEvent &e)
{
  if (e.keycode == _return) {
    std::vector<DialogButton>::const_iterator it, end = _buttons.end();
    for (it = _buttons.begin(); it != end; ++it)
      if (it->isDefault()) {
	_result = &(*it);
	hide();
	break;
      }
  } else if (e.keycode == _escape) {
    hide();
  }
}

void MessageDialog::clientMessageHandler(const XClientMessageEvent &e)
{
  EventHandler::clientMessageHandler(e);
  if (e.message_type == Property::atoms.wm_protocols &&
      static_cast<Atom>(e.data.l[0]) == Property::atoms.wm_delete_window)
    hide();
}

}
