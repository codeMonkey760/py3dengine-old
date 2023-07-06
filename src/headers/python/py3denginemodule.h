#ifndef PY3DENGINE_PY3DENGINEMODULE_H
#define PY3DENGINE_PY3DENGINEMODULE_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

extern int appendPy3dEngineModule();
extern int importPy3dEngineModule();
extern int initPy3dEngineObjects();
extern PyObject *getPy3dEngineModule();
extern void finalizePy3dEngineModule();

#endif
