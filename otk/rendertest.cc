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
  
  otk::RenderTexture tex;
  foo.setTexture(&tex);

  foo.show();
  
  app.run();

  printf("\n");
  return 0;
}
