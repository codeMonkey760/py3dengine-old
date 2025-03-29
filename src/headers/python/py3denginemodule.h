#ifndef PY3DENGINE_PY3DENGINEMODULE_H
#define PY3DENGINE_PY3DENGINEMODULE_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

extern PyObject *Py3dErr_SceneError;

extern int appendPy3dEngineExtModule();
extern int importPy3dEngineExtModule();
extern int initPy3dEngineExtObjects();
extern PyObject *getPy3dEngineExtModule();
extern void finalizePy3dEngineExtModule();

#endif
