$set 14 #main

$ #RCRequiresArg
# error: '-rc' requiere un argumento\n
$ #DISPLAYRequiresArg
# error: '-display' requiere un argumento\n
$ #WarnDisplaySet
# warning: no pude establecer la variable de entorno 'DISPLAY'\n
$ #Usage
# Blackbox %s : (c) 2001 - 2002 Sean 'Shaleh' Perry\n\
  \t\t\t 1997 - 2000, 2002 Brad Hughes\n\n\
  -display <cadena>\t\tusar la conexión al display.\n\
  -rc <cadena>\t\t\tusar otro archivo de recursos.\n\
  -version\t\t\tmostrar version y salir.\n\
  -help\t\t\t\tmostrar este texto de ayuda y salir.\n\n
$ #CompileOptions
# Opciones de Tiempo de Compilación:\n\
  Depuración:\t\t\t%s\n\
  Forma:\t\t\t%s\n\
  Simulación ordenada de colores de 8bpp:\t%s\n\n
