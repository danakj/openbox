extern "C" {
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
}

#include <iostream>
#include <string>
#include <vector>

const char *NAME = "xftlsfonts";
const char *VERSION = "1.0";

using std::string;
using std::cout;
using std::endl;

int main(int argc, char **argv) {
  if (argc > 1) {
    for (int i = 1; i < argc; ++i)
      if (string(argv[i]) == "-help" ||
          string(argv[i]) == "--help" ||
          string(argv[i]) == "-version" ||
          string(argv[i]) == "--version") {
        cout << NAME << " version " << VERSION << endl;
        cout << "Copyright (c) 2002, Ben Jansens <ben@orodu.net>" << endl;
        cout << endl;
        return 1;
      }
  }

  Display *display = XOpenDisplay(NULL);

  XftFontSet *set = XftListFonts(display, DefaultScreen(display),
                                 0, XFT_FAMILY, 0);

  cout << "Found " << set->nfont << " fonts:" << endl;

  for (int i = 0; i < set->nfont; ++i) {
    for (int e = 0; e < set->fonts[i]->num; ++e) {
//      if (string(set->fonts[i]->elts[e].object) != "family")
//        continue; // i just want font family names

      if (e > 0)
        cout << "  "; // indent after the first element
      cout << set->fonts[i]->elts[e].object << ": ";

      XftValueList *vallist = set->fonts[i]->elts[e].values;
      bool f = true;
      do {
        if (f)
          f = false;
        else
          cout << ", ";

        XftValue val = vallist->value;
        switch (val.type) {
        case XftTypeVoid:
          cout << "(void)";
          break;

        case XftTypeInteger:
          cout << val.u.i;
          break;

        case XftTypeDouble:
          cout << val.u.d;
          break;

        case XftTypeString:
          cout << val.u.s;
          break;

        case XftTypeBool:
          cout << val.u.b;
          break;

        case XftTypeMatrix:
          cout << "xx(" << val.u.m->xx << ") ";
          cout << "xy(" << val.u.m->xy << ") ";
          cout << "yx(" << val.u.m->yx << ") ";
          cout << "yy(" << val.u.m->yy << ")";
          break;
        }
      } while ((vallist = vallist->next));
      cout << endl;
    }
  }

  XFree(set);
  
  XCloseDisplay(display);
  return 0;
}
