#ifndef __window_h
#define __window_h

#include <X11/Xlib.h>

typedef enum {
    Window_Menu,
    Window_Slit,
    Window_Client,
    Window_Internal
} Window_InternalType;

typedef struct ObWindow {
    Window_InternalType type;
} ObWindow;

/* Wrapper for internal stuff. If its struct matches this then it can be used
   as an ObWindow */
typedef struct InternalWindow {
    ObWindow obwin;
    Window win;
} InternalWindow;

#define WINDOW_IS_MENU(win) (((ObWindow*)win)->type == Window_Menu)
#define WINDOW_IS_SLIT(win) (((ObWindow*)win)->type == Window_Slit)
#define WINDOW_IS_CLIENT(win) (((ObWindow*)win)->type == Window_Client)
#define WINDOW_IS_INTERNAL(win) (((ObWindow*)win)->type == Window_Internal)

struct Menu;
struct Slit;
struct Client;

#define WINDOW_AS_MENU(win) ((struct Menu*)win)
#define WINDOW_AS_SLIT(win) ((struct Slit*)win)
#define WINDOW_AS_CLIENT(win) ((struct Client*)win)
#define WINDOW_AS_INTERNAL(win) ((struct InternalWindow*)win)

#define MENU_AS_WINDOW(menu) ((ObWindow*)menu)
#define SLIT_AS_WINDOW(slit) ((ObWindow*)slit)
#define CLIENT_AS_WINDOW(client) ((ObWindow*)client)
#define INTERNAL_AS_WINDOW(intern) ((ObWindow*)intern)

Window window_top(ObWindow *self);
Window window_layer(ObWindow *self);

#endif
