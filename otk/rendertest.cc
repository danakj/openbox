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

  otk::RenderColor color(0, 0xff, 0x20, 0x20);
  otk::RenderTexture tex(false,
			 otk::RenderTexture::Flat,
			 false,
			 otk::RenderTexture::Solid,
			 false,
			 &color,
			 0,
			 0,
			 0);
  foo.setTexture(&tex);

  foo.show();
  
  app.run();

  printf("\n");
  return 0;
}
