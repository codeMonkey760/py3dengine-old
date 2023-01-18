#ifndef PY3DENGINE_PY3DMATHMODULE_H
#define PY3DENGINE_PY3DMATHMODULE_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdbool.h>

extern bool appendPy3dMathModule();
extern bool importPy3dMathModule();
extern bool initPy3dMathObjects();
extern PyObject *getPy3dMathModule();
extern void finalizePy3dMathModule();

#endif
