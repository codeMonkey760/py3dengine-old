#ifndef PY3DENGINE_PY3DLOGGERMODULE_H
#define PY3DENGINE_PY3DLOGGERMODULE_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdbool.h>

extern bool appendPy3dLoggerModule();
extern bool importPy3dLoggerModule();
extern PyObject *getPy3dLoggerModule();

#endif
