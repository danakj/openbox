#include "config.h"

#include "otk.hh"
#include "rendercontrol.hh"
#include "rendertexture.hh"

extern "C" {
#include <X11/Xlib.h>
}

#include <cstdio>

int main(int argc, char **argv)
{
  printf("\n");

  otk::Application app(argc, argv);
  otk::AppWidget foo(&app);
  foo.resize(600, 500);

  otk::RenderColor color(0, 0x96, 0xba, 0x86);
  otk::RenderColor color2(0, 0x5a, 0x72, 0x4c);
  otk::RenderColor colord(0, 0, 0, 0);
  otk::RenderColor colorl(0, 0xff, 0xff, 0xff);
  otk::RenderTexture tex(false,
			 otk::RenderTexture::Raised,
			 otk::RenderTexture::Bevel1,
			 false,
			 otk::RenderTexture::Vertical,
			 false,
			 &color,
                         &color2,
			 &colord,
			 &colorl,
			 0,
			 0);
  foo.setTexture(&tex);

  foo.show();
  
  app.run();

  printf("\n");
  return 0;
}
