// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __oblabelwidget_hh
#define   __oblabelwidget_hh

#include "widgetbase.hh"
#include "otk/widget.hh"
#include "otk/font.hh"
#include "otk/style.hh"

namespace ob {

class OBLabelWidget : public otk::OtkWidget, public OBWidget
{
private:
  void setTextures();
  const otk::BFont *_font;
  otk::BColor *_text_color;
  int _sidemargin;
  otk::Style::TextJustify _justify;
  std::string _text;
  //! Object used by Xft to render to the drawable
  XftDraw *_xftdraw;
  
public:
  OBLabelWidget(otk::OtkWidget *parent, OBWidget::WidgetType type);
  virtual ~OBLabelWidget();

  virtual void setStyle(otk::Style *style);

  virtual void adjust();

  virtual void focus();
  virtual void unfocus();

  virtual void update();

  inline const std::string &text() const { return _text; }
  void setText(const std::string &text);
};

}

#endif // __oblabelwidget_hh
