#include "button.hh"

namespace otk {

OtkButton::OtkButton(OtkWidget *parent)
  : OtkWidget(parent), _text(""), _pressed(false),
    _unfocus_tx(OtkWidget::getStyle()->getButtonUnfocus())
{
}

OtkButton::~OtkButton()
{

}

void OtkButton::setText(const std::string &text)
{
  std::string a = text;
}

void OtkButton::press(void)
{

}

void OtkButton::release(void)
{

}

}
