#ifndef PY3DENGINE_PY3DENGINEMODULE_H
#define PY3DENGINE_PY3DENGINEMODULE_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdbool.h>

extern bool appendPy3dEngineModule();
extern bool importPy3dEngineModule();
extern bool initPy3dEngineObjects();
extern PyObject *getPy3dEngineModule();
extern void finalizePy3dEngineModule();

#endif
