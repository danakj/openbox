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
# Blackbox %s : (c) 2001 - 2002 Sean 'Shaleh' Perry\n\
  \t\t\t 1997 - 2000, 2002 Brad Hughes\n\n\
  -display <string>\t\tuse display connection.\n\
  -rc <string>\t\t\tuse alternate resource file.\n\
  -menu <string>\t\tuse alternate menu file.\n\
  -version\t\t\tdisplay version and exit.\n\
  -help\t\t\t\tdisplay this help text and exit.\n\n
$ #CompileOptions
# Compile time options:\n\
  Debugging\t\t\t%s\n\
  Shape:\t\t\t%s\n\
  Slit:\t\t\t\t%s\n\
  Event Clobbering:\t\t%s\n\
  8bpp Ordered Dithering:\t%s\n\n
