// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "python.hh"
#include "openbox.hh"
#include "actions.hh"
#include "python.hh"
#include "bindings.hh"
#include "otk/display.hh"
#include "otk/util.hh"

extern "C" {
#include <Python.h>

#include "gettext.h"
#define _(str) gettext(str)
}

namespace ob {

void python_init(char *argv0)
{
  // start the python engine
  Py_SetProgramName(argv0);
  Py_Initialize();
  // prepend the openbox directories for python scripts to the sys path
  PyRun_SimpleString("import sys");
  PyRun_SimpleString("sys.path.insert(0, '" SCRIPTDIR "')");
  PyRun_SimpleString(const_cast<char*>(("sys.path.insert(0, '" +
                                        otk::expandTilde("~/.openbox/python") +
                                        "')").c_str()));
}

void python_destroy()
{
  Py_Finalize();
}

int python_exec(const std::string &path)
{
  FILE *rcpyfd = fopen(path.c_str(), "r");
  if (!rcpyfd) {
    fprintf(stderr, _("Unabled to open python file %s\n"), path.c_str());
    return 1;
  }

  //PyRun_SimpleFile(rcpyfd, const_cast<char*>(path.c_str()));

  PyObject *module = PyImport_AddModule("__main__");
  assert(module);
  PyObject *dict = PyModule_GetDict(module);
  assert(dict);
  PyObject *result = PyRun_File(rcpyfd, const_cast<char*>(path.c_str()),
                                Py_file_input, dict, dict);
  int ret = result == NULL ? 2 : 0;
  if (result == NULL)
    PyErr_Print();
  
  Py_XDECREF(result);
    
  Py_DECREF(dict);

  fclose(rcpyfd);
  return ret;
}

}
