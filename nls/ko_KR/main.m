$set 14 #main

$ #RCRequiresArg
# error: '-rc' requires an argument\n
$ #MENURequiresArg
# error: '-menu' requires an argument\n
$ #DISPLAYRequiresArg
# error: '-display' requires an argument\n
$ #WarnDisplaySet
# warning: could not set environment variable 'DISPLAY'\n
$ #Usage
# Openbox %s : (c) 2002 - 2002 Ben Jansens\n\
                    2001 - 2002 Sean 'Shaleh' Perry\n\
                    1997 - 2000, 2002 Brad Hughes\n\n\
  -display <string>\t\tuse display connection.\n\
  -rc <string>\t\t\tuse alternate resource file.\n\
  -version\t\t\tdisplay version and exit.\n\
  -help\t\t\t\tdisplay this help text and exit.\n\n
$ #CompileOptions
# Compile time options:\n\
  Debugging\t\t\t%s\n\
  Shape:\t\t\t%s\n\
  8bpp Ordered Dithering:\t%s\n\n
