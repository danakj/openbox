#ifndef __hooks_h
#define __hooks_h

#include "clientwrap.h"
#include <Python.h>

void hooks_startup();
void hooks_shutdown();

struct HookObject;

struct HookObject *hooks_create(char *name);

struct HookObject *hook_startup;
struct HookObject *hook_shutdown;
struct HookObject *hook_visibledesktop;
struct HookObject *hook_numdesktops;
struct HookObject *hook_desktopnames;
struct HookObject *hook_showdesktop;
struct HookObject *hook_screenconfiguration;
struct HookObject *hook_screenarea;
struct HookObject *hook_managed;
struct HookObject *hook_closed;
struct HookObject *hook_bell;
struct HookObject *hook_urgent;
struct HookObject *hook_pointerenter;
struct HookObject *hook_pointerleave;
struct HookObject *hook_focused;
struct HookObject *hook_requestactivate;
struct HookObject *hook_title;
struct HookObject *hook_desktop;
struct HookObject *hook_iconic;
struct HookObject *hook_shaded;
struct HookObject *hook_maximized;
struct HookObject *hook_fullscreen;
struct HookObject *hook_visible;
struct HookObject *hook_configuration;

#define HOOKFIRE(hook, ...)                              \
{                                                        \
    PyObject *args = Py_BuildValue(__VA_ARGS__);         \
    g_assert(args != NULL);                              \
    hooks_fire(hook_##hook, args);                       \
    Py_DECREF(args);                                     \
}

#define HOOKFIRECLIENT(hook, client)                     \
{                                                        \
    hooks_fire_client(hook_##hook, client);              \
}

void hooks_fire(struct HookObject *hook, PyObject *args);

void hooks_fire_client(struct HookObject *hook, struct Client *client);

#endif
