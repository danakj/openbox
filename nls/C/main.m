$set 14 #main

$ #RCRequiresArg
# error: '-rc' requiere un argumento
$ #DISPLAYRequiresArg
# error: '-display' requiere un argumento
$ #WarnDisplaySet
# cuidado: no se puede establecer la variable de ambiente 'DISPLAY'
$ #Usage
# Openbox %s : (c) 2002 - 2002 Ben Jansens\n\
  \t\t\t 2001 - 2002, Sean 'Shaleh' Perry\n\n\
  \t\t\t 1997 - 2000, 2002 Brad Hughes\n\n\
  -display <string> conexión de despliegue.\n\
  -rc <string>      archivo alternativo de recuros.\n\
  -version          mostrar la versión y cerrar.\n\
  -help             mostrar este texto de ayuda y cerrar.\n\n
$ #CompileOptions
# Opciones durante la compilación:\n\
  Información extra para depuración:               %s\n\
  Forma:                                           %s\n\
  Slit:                                            %s\n\
  Event Clobbering:                                %s\n\
  8bpp simulación ordenada de colores en imágenes: %s\n\n
