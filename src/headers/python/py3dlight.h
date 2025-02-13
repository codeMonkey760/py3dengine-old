#ifndef PY3DLIGHT_H
#define PY3DLIGHT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

struct Py3dLight;

extern int PyInit_Py3dLight(PyObject *module);
extern int Py3dLight_FindCtor(PyObject *module);
extern void Py3dLight_FinalizeCtor();
extern struct Py3dLight *Py3dLight_New();
extern int Py3dLight_Check(PyObject *obj);

extern PyObject *Py3dLight_Parse(struct Py3dLight *self, PyObject *args, PyObject *kwds);

#endif
