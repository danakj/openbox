#include "otk.hh"
#include "rendercontrol.hh"

#include <stdio.h>

int main(int argc, char **argv)
{
  printf("\n");

  otk::Application app(argc, argv);
  otk::AppWidget foo(&app);
  foo.resize(600, 500);
  foo.show();
  
  otk::RenderControl *rc = otk::RenderControl::getRenderControl(0);

  rc->render(foo.window());

  app.run();

  delete rc;

  printf("\n");
  return 0;
}
