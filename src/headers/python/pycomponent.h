#ifndef PY3DENGINE_PYCOMPONENT_H
#define PY3DENGINE_PYCOMPONENT_H

#include <stdbool.h>
#include <structmember.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

extern bool PyInit_Py3dComponent(PyObject *module);
extern bool findPyComponentCtor(PyObject *module);
extern PyObject *createPyComponent();
extern void finalizePyComponentCtor();

#endif
