#ifndef __window_h
#define __window_h

#include <X11/Xlib.h>
#include <glib.h>

typedef enum {
    Window_Menu,
    Window_Dock,
    Window_DockApp, /* used for events but not stacking */
    Window_Client,
    Window_Decoration,
    Window_Internal /* used for stacking but not events */
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
#define WINDOW_IS_DOCK(win) (((ObWindow*)win)->type == Window_Dock)
#define WINDOW_IS_DOCKAPP(win) (((ObWindow*)win)->type == Window_DockApp)
#define WINDOW_IS_CLIENT(win) (((ObWindow*)win)->type == Window_Client)
#define WINDOW_IS_INTERNAL(win) (((ObWindow*)win)->type == Window_Internal)
#define WINDOW_IS_DECORATION(win) (((ObWindow*)win)->type == Window_Decoration)

struct Menu;
struct Dock;
struct DockApp;
struct Client;
struct Decoration;

#define WINDOW_AS_MENU(win) ((struct Menu*)win)
#define WINDOW_AS_DOCK(win) ((struct Dock*)win)
#define WINDOW_AS_DOCKAPP(win) ((struct DockApp*)win)
#define WINDOW_AS_CLIENT(win) ((struct Client*)win)
#define WINDOW_AS_INTERNAL(win) ((struct InternalWindow*)win)
#define WINDOW_AS_DECORATION(win) ((struct FrameDecor *)win)

#define MENU_AS_WINDOW(menu) ((ObWindow*)menu)
#define DOCK_AS_WINDOW(dock) ((ObWindow*)dock)
#define DOCKAPP_AS_WINDOW(dockapp) ((ObWindow*)dockapp)
#define CLIENT_AS_WINDOW(client) ((ObWindow*)client)
#define INTERNAL_AS_WINDOW(intern) ((ObWindow*)intern)

extern GHashTable *window_map;

void window_startup();
void window_shutdown();

Window window_top(ObWindow *self);
Window window_layer(ObWindow *self);

#endif
