$set 14 #main

$ #RCRequiresArg
# 错误: '-rc' 需要参数\n
$ #MENURequiresArg
# 错误: '-menu' 需要参数\n
$ #DISPLAYRequiresArg
# 错误: '-display' 需要参数\n
$ #WarnDisplaySet
# 警告: 不能设置环境变量 'DISPLAY'\n
$ #Usage
# Openbox %s : (c) 2002 - 2002 Ben Jansens\n\
                    2001 - 2002 Sean 'Shaleh' Perry\n\
                    1997 - 2000, 2002 Brad Hughes\n\n\
  -display <string>\t\t使用显示连接.\n\
  -rc <string>\t\t\t使用其他的资源文件.\n\
  -menu <string>\t\tuse alternate menu file.\n\
  -version\t\t\t显示版本.\n\
  -help\t\t\t\t显示这个帮助.\n\n
$ #CompileOptions
# 编译选项:\n\
  Debugging\t\t\t%s\n\
  Shape:\t\t\t%s\n\
  Xft:\t\t\t\t%s\n\
  Xinerama:\t\t\t%s\n\
  8bpp Ordered Dithering:\t%s\n\n
