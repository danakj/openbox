#include <Python.h>
#include <glib.h>

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

void python_startup()
{
    PyObject *sys, *sysdict, *syspath, *path1, *path2;
    char *home, *homescriptdir;

    Py_Initialize();

    /* fix up the system path */

    sys = PyImport_ImportModule((char*)"sys"); /* new */
    sysdict = PyModule_GetDict(sys); /* borrowed */
    syspath = PyDict_GetItemString(sysdict, (char*)"path"); /* borrowed */

    path1 = PyString_FromString(SCRIPTDIR); /* new */
    PyList_Insert(syspath, 0, path1);
    Py_DECREF(path1);

    home = getenv("HOME");
    if (home != NULL) {
	homescriptdir = g_strdup_printf("%s/.openbox", home);
	path2 = PyString_FromString(homescriptdir); /* new */
	g_free(homescriptdir);

	PyList_Insert(syspath, 0, path2);
	Py_DECREF(path2);
    } else
	g_warning("Failed to read the $HOME environment variable");

    Py_DECREF(sys);
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
