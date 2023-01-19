#ifndef PY3DENGINE_VECTOR3_H
#define PY3DENGINE_VECTOR3_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include <stdbool.h>

extern bool PyInit_Py3dVector3(PyObject *module);
extern bool Py3dPy3dVector3_FindCtor(PyObject *module);
extern void Py3dVector3_FinalizeCtor();
extern struct Py3dVector3 *Py3dVector3_New();

#endif