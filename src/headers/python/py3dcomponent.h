#ifndef PY3DENGINE_PY3DCOMPONENT_H
#define PY3DENGINE_PY3DCOMPONENT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <stdbool.h>

struct Py3dComponent {
    PyObject_HEAD
    PyObject *name;
    struct GameObject *owner;
};

extern PyTypeObject Py3dComponent_Type;

extern bool PyInit_Py3dComponent(PyObject *module);
extern bool Py3dComponent_IsComponent(PyObject *pyObj);

#endif
