// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __widgetbase_hh
#define   __widgetbase_hh

#include "python.hh"

namespace ob {

class WidgetBase {
public:
  enum WidgetType {
    Type_Frame,
    Type_Titlebar,
    Type_Handle,
    Type_Plate,
    Type_Label,
    Type_MaximizeButton,
    Type_CloseButton,
    Type_IconifyButton,
    Type_AllDesktopsButton,
    Type_LeftGrip,
    Type_RightGrip,
    Type_Client,
    Type_Root
  };

private:
  WidgetType _type;

public:
  WidgetBase(WidgetType type) : _type(type) {}
  
  inline WidgetType type() const { return _type; }

  inline MouseContext::MC mcontext() const {
    switch (_type) {
    case Type_Frame:
      return MouseContext::Frame;
    case Type_Titlebar:
      return MouseContext::Titlebar;
    case Type_Handle:
      return MouseContext::Handle;
    case Type_Plate:
      return MouseContext::Window;
    case Type_Label:
      return MouseContext::Titlebar;
    case Type_MaximizeButton:
      return MouseContext::MaximizeButton;
    case Type_CloseButton:
      return MouseContext::CloseButton;
    case Type_IconifyButton:
      return MouseContext::IconifyButton;
    case Type_AllDesktopsButton:
      return MouseContext::AllDesktopsButton;
    case Type_LeftGrip:
      return MouseContext::Grip;
    case Type_RightGrip:
      return MouseContext::Grip;
    case Type_Client:
      return MouseContext::Window;
    case Type_Root:
      return MouseContext::Root;
    default:
      assert(false); // unhandled type
    }
  }
};

}

#endif // __widgetbase_hh
