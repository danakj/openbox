$set 14 #main

$ #RCRequiresArg
# ошибка: '-rc' требует наличие аргумента\n
$ #MENURequiresArg
# ошибка: '-menu' требует наличие аргумента\n
$ #DISPLAYRequiresArg
# ошибка: '-display' требует наличие аргумента\n
$ #WarnDisplaySet
# предупреждение: невозможно установить переменную окружения 'DISPLAY'\n
$ #Usage
# Openbox %s : (c) 2002 - 2002 Ben Jansens\n\
                    2001 - 2002 Sean 'Shaleh' Perry\n\
                    1997 - 2000, 2002 Brad Hughes\n\n\
  -display <string>\t\tиспользовать заданный дисплей.\n\
  -rc <string>\t\t\tиспользовать альтернативный файл ресурсов.\n\
  -menu <string>\t\tuse alternate menu file.\n\
  -version\t\t\tвывести номер версии и выйти.\n\
  -help\t\t\t\tвывести эту подсказку и выйти.\n\n
$ #CompileOptions
# Compile time options:\n\
  Debugging\t\t\t%s\n\
  Shape:\t\t\t%s\n\
  Slit:\t\t\t\t%s\n\
  Event Clobbering:\t\t%s\n\
  8bpp Ordered Dithering:\t%s\n\n
