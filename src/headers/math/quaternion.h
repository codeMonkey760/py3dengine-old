#ifndef PY3DENGINE_QUATERNION_H
#define PY3DENGINE_QUATERNION_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include <stdbool.h>

struct Py3dQuaternion {
    PyObject_HEAD
    float elements[4];
};

extern PyTypeObject Py3dQuaternion_Type;

extern bool PyInit_Py3dQuaternion(PyObject *module);
extern bool Py3dQuaternion_FindCtor(PyObject *module);
extern void Py3dQuaternion_FinalizeCtor();
extern struct Py3dQuaternion *Py3dQuaternion_New(float x, float y, float z, float w);

#endif
