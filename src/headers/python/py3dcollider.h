#ifndef PY3DENGINE_PY3DCOLLIDER_H
#define PY3DENGINE_PY3DCOLLIDER_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <ode/ode.h>

#include "py3dcomponent.h"

struct Py3dCollider {
    struct Py3dComponent base;
    dGeomID geomId;
    int isTrigger; //generate collision messages only when true, don't physically collide
};

extern int PyInit_Py3dCollider(PyObject *module);
extern int Py3dCollider_FindCtor(PyObject *module);
extern void Py3dCollider_FinalizeCtor();
extern struct Py3dCollider *Py3dCollider_New();
extern int Py3dCollider_Check(PyObject *obj);

#endif
