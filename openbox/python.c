#include <Python.h>
#include <glib.h>

static PyMethodDef ObMethods[] = {
    { NULL, NULL, 0, NULL }
};

static PyMethodDef InputMethods[] = {
    { NULL, NULL, 0, NULL }
};

void python_startup()
{
    PyObject *sys, *sysdict, *syspath, *path1, *path2;
    char *homescriptdir;

    Py_Initialize();

    /* fix up the system path */

    sys = PyImport_ImportModule("sys"); /* new */
    sysdict = PyModule_GetDict(sys); /* borrowed */
    syspath = PyDict_GetItemString(sysdict, "path"); /* borrowed */

    path1 = PyString_FromString(SCRIPTDIR); /* new */
    PyList_Insert(syspath, 0, path1);
    Py_DECREF(path1);

    homescriptdir = g_build_filename(g_get_home_dir(), ".openbox", NULL);
    path2 = PyString_FromString(homescriptdir); /* new */
    PyList_Insert(syspath, 0, path2);
    Py_DECREF(path2);
    g_free(homescriptdir);

    Py_DECREF(sys);

    /* create the 'ob' module */
    Py_InitModule("ob", ObMethods);

    /* create the 'input' module */
    Py_InitModule("input", InputMethods);
}

void python_shutdown()
{
    Py_Finalize();
}

gboolean python_import(char *module)
{
    PyObject *mod;

    mod = PyImport_ImportModule(module); /* new */
    if (mod == NULL) {
	PyErr_Print();
	return FALSE;
    }
    Py_DECREF(mod);
    return TRUE;
}
