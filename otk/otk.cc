#include "display.hh"
#include "timer.hh"
#include "renderstyle.hh"
#include "property.hh"

namespace otk {

void initialize()
{
  new Display();
  Timer::initialize();
  RenderColor::initialize();
  RenderStyle::initialize();
  Property::initialize();
}

void destroy()
{
  RenderStyle::destroy();
  RenderColor::destroy();
  Timer::destroy();
  delete display;
}

}
