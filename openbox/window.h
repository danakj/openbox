#ifndef __window_h
#define __window_h

#include <X11/Xlib.h>

typedef enum {
    Window_Menu,
    Window_Slit,
    Window_Client
} Window_InternalType;

typedef struct ObWindow {
    Window_InternalType type;
} ObWindow;

#define WINDOW_IS_MENU(win) (((ObWindow*)win)->type == Window_Menu)
#define WINDOW_IS_SLIT(win) (((ObWindow*)win)->type == Window_Slit)
#define WINDOW_IS_CLIENT(win) (((ObWindow*)win)->type == Window_Client)

struct Menu;
struct Slit;
struct Client;

#define WINDOW_AS_MENU(win) ((struct Menu*)win)
#define WINDOW_AS_SLIT(win) ((struct Slit*)win)
#define WINDOW_AS_CLIENT(win) ((struct Client*)win)

#define MENU_AS_WINDOW(menu) ((ObWindow*)menu)
#define SLIT_AS_WINDOW(slit) ((ObWindow*)slit)
#define CLIENT_AS_WINDOW(client) ((ObWindow*)client)

Window window_top(ObWindow *self);
Window window_layer(ObWindow *self);

#endif
