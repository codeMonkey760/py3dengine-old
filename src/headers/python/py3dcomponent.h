#ifndef PY3DENGINE_PY3DCOMPONENT_H
#define PY3DENGINE_PY3DCOMPONENT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <stdbool.h>

struct Py3dGameObject;
struct RenderingContext;
struct ResourceManager;

struct Py3dComponent {
    PyObject_HEAD
    PyObject *name;
    struct Py3dGameObject *owner;
};

extern PyTypeObject Py3dComponent_Type;

extern bool PyInit_Py3dComponent(PyObject *module);
extern bool Py3dComponent_IsComponent(PyObject *pyObj);
extern PyObject *Py3dComponent_GetName(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored));
extern PyObject *Py3dComponent_GetOwner(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored));

extern bool Py3dComponent_CallParse(struct Py3dComponent *component, PyObject *data, struct ResourceManager *rm);

#endif
