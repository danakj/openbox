#include "otk.hh"
#include "rendercontrol.hh"
#include "rendertexture.hh"

#include <stdio.h>
#include <X11/Xlib.h>

int main(int argc, char **argv)
{
  printf("\n");

  otk::Application app(argc, argv);
  otk::AppWidget foo(&app);
  foo.resize(600, 500);
  foo.show();
  
  otk::RenderControl *rc = otk::RenderControl::getRenderControl(0);

  otk::RenderTexture tex;
  
  rc->drawBackground(&foo, tex);
  XSetWindowBackgroundPixmap(**otk::display, foo.window(), foo.pixmap());
  XClearWindow(**otk::display, foo.window());
  
  app.run();

  delete rc;

  printf("\n");
  return 0;
}
