const char *NAME = "xftlsfonts";
const char *VERSION = "1.0";

#ifdef    HAVE_CONFIG_H
#include "../config.h"
#endif // HAVE_CONFIG_H

#ifndef   XFT
#include <iostream>
using std::cout;
using std::endl;

int main(int, char **) {
  cout << NAME << " version " << VERSION << endl;
  cout << "Copyright (c) 2002, Ben Jansens <ben@orodu.net>" << endl;
  cout << endl;
  cout << "Openbox was built without support for Xft fonts. This utility must"
    << endl;
  cout << "must be built with Xft support in order to function." << endl;
}
#else  // XFT
extern "C" {
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
}

#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::cout;
using std::endl;

int main(int argc, char **argv) {
  if (argc > 1)
    for (int i = 1; i < argc; ++i)
      if (string(argv[i]) == "-help" ||
          string(argv[i]) == "--help" ||
          string(argv[i]) == "-version" ||
          string(argv[i]) == "--version") {
        cout << NAME << " version " << VERSION << endl;
        cout << "Copyright (c) 2002, Ben Jansens <ben@orodu.net>" << endl;
        cout << endl;
        cout << "Usage: " << argv[0] << " [options]" << endl;
        cout << "    -style     Show possible styles for each font" << endl;
        cout << "    -slant     Show the slant for each font" << endl;
        cout << "    -weight    Show the weight for each font" << endl;
        cout << "    -file      Show which files contain each font" << endl;
        cout << endl;
        return 1;
      }

  Display *display = XOpenDisplay(NULL);
  if (! display) {
    cout << "Failed to open connection to X display\n";
    return 2;
  }

  XftObjectSet *obj = XftObjectSetCreate();
  if (! obj) {
    cout << "Failed to create an XftObjectSet\n";
    return 2;
  }

  XftObjectSetAdd(obj, XFT_FAMILY);

  if (argc > 1)
    for (int i = 1; i < argc; ++i) {
      if (string(argv[i]) == "-style") XftObjectSetAdd(obj, XFT_STYLE);
      else if (string(argv[i]) == "-file") XftObjectSetAdd(obj, XFT_FILE);
      else if (string(argv[i]) == "-slant") XftObjectSetAdd(obj, XFT_SLANT);
      else if (string(argv[i]) == "-weight") XftObjectSetAdd(obj, XFT_WEIGHT);
    }

  XftPattern *pat = XftPatternCreate();
  if (! pat) {
    cout << "Failed to create an XftPattern\n";
    exit(2);
  }

  XftFontSet *set = XftListFontsPatternObjects(display, DefaultScreen(display),
                                               pat, obj);
  if (! set) {
    cout << "Failed to find a matching XftFontSet\n";
    exit(2);
  }
 
  XFree(pat);
  XFree(obj);

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

        default:
          break;
        }
      } while ((vallist = vallist->next));
      cout << endl;
    }
  }
  
  cout << endl << "Found " << set->nfont << " matches." << endl;

  XFree(set);
  
  XCloseDisplay(display);
  return 0;
}
#endif // XFT

