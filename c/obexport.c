#include "obexport.h"
#include <Python.h>
#include <glib.h>

static PyMethodDef obMethods[] = {
    { NULL, NULL, 0, NULL }
};

#define ADD_INT_CONST(n) (PyModule_AddIntConstant(ob, #n, n))

void obexport_startup()
{
    PyObject *ob, *obdict;

    Py_InitModule("ob", obMethods);

    /* get the ob module/dict */
    ob = PyImport_ImportModule("ob"); /* new */
    g_assert(ob != NULL);
    obdict = PyModule_GetDict(ob); /* borrowed */
    g_assert(obdict != NULL);

    /* define all the constants! */

    /* State */
    ADD_INT_CONST(State_Starting);
    ADD_INT_CONST(State_Exiting);
    ADD_INT_CONST(State_Running);

    /* Corner */
    ADD_INT_CONST(Corner_TopLeft);
    ADD_INT_CONST(Corner_TopRight);
    ADD_INT_CONST(Corner_BottomLeft);
    ADD_INT_CONST(Corner_BottomRight);

    /* Orientation */
    ADD_INT_CONST(Orientation_Horz);
    ADD_INT_CONST(Orientation_Vert);

    /* Gravity */
    ADD_INT_CONST(Gravity_Forget);
    ADD_INT_CONST(Gravity_NE);
    ADD_INT_CONST(Gravity_N);
    ADD_INT_CONST(Gravity_NW);
    ADD_INT_CONST(Gravity_W);
    ADD_INT_CONST(Gravity_SW);
    ADD_INT_CONST(Gravity_S);
    ADD_INT_CONST(Gravity_SE);
    ADD_INT_CONST(Gravity_E);
    ADD_INT_CONST(Gravity_Center);
    ADD_INT_CONST(Gravity_Static);

    /* WindowType */
    ADD_INT_CONST(Type_Desktop);
    ADD_INT_CONST(Type_Dock);
    ADD_INT_CONST(Type_Toolbar);
    ADD_INT_CONST(Type_Menu);
    ADD_INT_CONST(Type_Utility);
    ADD_INT_CONST(Type_Splash);
    ADD_INT_CONST(Type_Dialog);
    ADD_INT_CONST(Type_Normal);

    /* Function */
    ADD_INT_CONST(Func_Resize);
    ADD_INT_CONST(Func_Move);
    ADD_INT_CONST(Func_Iconify);
    ADD_INT_CONST(Func_Maximize);
    ADD_INT_CONST(Func_Shade);
    ADD_INT_CONST(Func_Fullscreen);
    ADD_INT_CONST(Func_Close);

    /* Decoration */
    ADD_INT_CONST(Decor_Titlebar);
    ADD_INT_CONST(Decor_Handle);
    ADD_INT_CONST(Decor_Border);
    ADD_INT_CONST(Decor_Icon);
    ADD_INT_CONST(Decor_Iconify);
    ADD_INT_CONST(Decor_Maximize);
    ADD_INT_CONST(Decor_AllDesktops);
    ADD_INT_CONST(Decor_Close);

    /* StackLayer */
    ADD_INT_CONST(Layer_Icon);
    ADD_INT_CONST(Layer_Desktop);
    ADD_INT_CONST(Layer_Below);
    ADD_INT_CONST(Layer_Normal);
    ADD_INT_CONST(Layer_Above);
    ADD_INT_CONST(Layer_Top);
    ADD_INT_CONST(Layer_Fullscreen);
    ADD_INT_CONST(Layer_Internal);

    /* EventType */
    ADD_INT_CONST(Logical_EnterWindow);
    ADD_INT_CONST(Logical_LeaveWindow);
    ADD_INT_CONST(Logical_NewWindow);
    ADD_INT_CONST(Logical_CloseWindow);
    ADD_INT_CONST(Logical_Startup);
    ADD_INT_CONST(Logical_Shutdown);
    ADD_INT_CONST(Logical_RequestActivate);
    ADD_INT_CONST(Logical_Focus);
    ADD_INT_CONST(Logical_Bell);
    ADD_INT_CONST(Logical_UrgentWindow);
    ADD_INT_CONST(Logical_WindowShow);
    ADD_INT_CONST(Logical_WindowHide);
    ADD_INT_CONST(Pointer_Press);
    ADD_INT_CONST(Pointer_Release);
    ADD_INT_CONST(Pointer_Motion);
    ADD_INT_CONST(Key_Press);
    ADD_INT_CONST(Key_Release);

    Py_DECREF(ob);
}

void obexport_shutdown()
{
}
