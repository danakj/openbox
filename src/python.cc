// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "python.hh"
#include "openbox.hh"
#include "actions.hh"
#include "python.hh"
#include "bindings.hh"
#include "otk/display.hh"
#include "otk/util.hh"

namespace ob {

static PyObject *obdict = NULL;

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
  //PyRun_SimpleString("import ob; import otk; import config;");
  PyRun_SimpleString("import config;");
  // set up convenience global variables
  //PyRun_SimpleString("ob.openbox = ob.Openbox_instance()");
  //PyRun_SimpleString("otk.display = otk.Display_instance()");

  // set up access to the python global variables
  PyObject *obmodule = PyImport_AddModule("config");
  obdict = PyModule_GetDict(obmodule);
}

void python_destroy()
{
  Py_Finalize();
}

bool python_exec(const std::string &path)
{
  FILE *rcpyfd = fopen(path.c_str(), "r");
  if (!rcpyfd) {
    printf("Failed to load python file %s\n", path.c_str());
    return false;
  }
  PyRun_SimpleFile(rcpyfd, const_cast<char*>(path.c_str()));
  fclose(rcpyfd);
  return true;
}

bool python_get_long(const char *name, long *value)
{
  PyObject *val = PyDict_GetItemString(obdict, const_cast<char*>(name));
  if (!(val && PyInt_Check(val))) return false;
  
  *value = PyInt_AsLong(val);
  return true;
}

bool python_get_string(const char *name, otk::ustring *value)
{
  PyObject *val = PyDict_GetItemString(obdict, const_cast<char*>(name));
  if (!(val && PyString_Check(val))) return false;
  
  *value = PyString_AsString(val);
  return true;
}

bool python_get_stringlist(const char *name, std::vector<otk::ustring> *value)
{
  PyObject *val = PyDict_GetItemString(obdict, const_cast<char*>(name));
  if (!(val && PyList_Check(val))) return false;

  for (int i = 0, end = PyList_Size(val); i < end; ++i) {
    PyObject *str = PyList_GetItem(val, i);
    if (PyString_Check(str))
      value->push_back(PyString_AsString(str));
  }
  return true;
}

}
