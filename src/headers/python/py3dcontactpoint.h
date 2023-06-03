#ifndef PY3DENGINE_PY3DCONTACTPOINT_H
#define PY3DENGINE_PY3DCONTACTPOINT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <ode/ode.h>

struct Py3dContactPoint {
    PyObject_HEAD
    PyObject *position;
    PyObject *normal;
    PyObject *depth;
};

extern PyTypeObject Py3dContactPoint_Type;

extern int PyInit_Py3dContactPoint(PyObject *module);
extern int Py3dContactPoint_FindCtor(PyObject *module);
extern void Py3dContactPoint_FinalizeCtor();
extern struct Py3dContactPoint *Py3dContactPoint_New(dContactGeom *odeContactPoint);
extern int Py3dContactPoint_Check(PyObject *obj);

extern PyObject *Py3dContactPoint_GetPosition(struct Py3dContactPoint *self, void *closure);
extern PyObject *Py3dContactPoint_GetNormal(struct Py3dContactPoint *self, void *closure);
extern PyObject *Py3dContactPoint_GetDepth(struct Py3dContactPoint *self, void *closure);

#endif
