#ifndef PY3DENGINE_PYTHON_UTIL_H
#define PY3DENGINE_PYTHON_UTIL_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

struct Py3dComponent;
struct Py3dGameObject;

extern void handleException();
extern void forceGarbageCollection();
extern void dumpPythonObjects();
extern PyObject *Py3d_GetTypeFromTuple(PyObject *tuple, Py_ssize_t index, PyTypeObject *type);
extern struct Py3dGameObject *Py3d_GetComponentOwner(struct Py3dComponent *component);

#endif
