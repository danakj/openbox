$set 14 #main

$ #RCRequiresArg
# エラー: '-rc' オプションは引数を必要とします\n
$ #MENURequiresArg
# エラー: '-menu' オプションは引数を必要とします\n
$ #DISPLAYRequiresArg
# エラー: '-display' オプションは引数を必要とします\n
$ #WarnDisplaySet
# 警告: 環境変数 'DISPLAY' を設定できませんでした\n
$ #Usage
# Openbox %s : (c) 2002 - 2002 Ben Jansens\n\
                    2001 - 2002 Sean 'Shaleh' Perry\n\
                    1997 - 2000, 2002 Brad Hughes\n\n\
  -display <string>\t\t指定ディスプレイに接続\n\
  -rc <string>\t\t\t代りのリソースファイルを使用\n\
  -menu <string>\t\tuse alternate menu file.\n\
  -version\t\t\tバージョン情報を表示し、終了\n\
  -help\t\t\t\tこのヘルプを表示し、終了\n\n
$ #CompileOptions
# コンパイル時オプション:\n\
  Debugging\t\t\t%s\n\
  Shape:\t\t\t%s\n\
  Xft:\t\t\t\t%s\n\
  Xinerama:\t\t\t%s\n\
  8bpp Ordered Dithering:\t%s\n\n
