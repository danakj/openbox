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
    Type_StickyButton,
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

  inline MouseContext mcontext() const {
    switch (_type) {
    case Type_Frame:
      return MC_Frame;
    case Type_Titlebar:
      return MC_Titlebar;
    case Type_Handle:
      return MC_Handle;
    case Type_Plate:
      return MC_Window;
    case Type_Label:
      return MC_Titlebar;
    case Type_MaximizeButton:
      return MC_MaximizeButton;
    case Type_CloseButton:
      return MC_CloseButton;
    case Type_IconifyButton:
      return MC_IconifyButton;
    case Type_StickyButton:
      return MC_StickyButton;
    case Type_LeftGrip:
      return MC_Grip;
    case Type_RightGrip:
      return MC_Grip;
    case Type_Client:
      return MC_Window;
    case Type_Root:
      return MC_Root;
    default:
      assert(false); // unhandled type
    }
  }
};

}

#endif // __widgetbase_hh
