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

}
