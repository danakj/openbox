// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __obwidget_hh
#define   __obwidget_hh

namespace ob {

class OBWidget {
public:
  enum WidgetType {
    Type_Titlebar,
    Type_Handle,
    Type_Plate
  };

private:
  WidgetType _type;

public:
  OBWidget(WidgetType type) : _type(type) {}
  
  inline WidgetType type() const { return _type; }
};

}

#endif // __obwidget_hh
