#ifndef PY3DENGINE_PYCOMPONENT_H
#define PY3DENGINE_PYCOMPONENT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <stdbool.h>

struct Py3dComponent {
    PyObject_HEAD
    PyObject *name;
    struct GameObject *owner;
};

extern bool PyInit_Py3dComponent(PyObject *module);

#endif
