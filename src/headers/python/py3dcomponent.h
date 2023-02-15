#ifndef PY3DENGINE_PY3DCOMPONENT_H
#define PY3DENGINE_PY3DCOMPONENT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <stdbool.h>

struct Py3dGameObject;
struct ResourceManager;

struct Py3dComponent {
    PyObject_HEAD
    PyObject *name;
    PyObject *owner;
};

extern PyTypeObject Py3dComponent_Type;

extern int PyInit_Py3dComponent(PyObject *module);
extern int Py3dComponent_Check(PyObject *pyObj);
extern void Py3dComponent_Dealloc(struct Py3dComponent *self);
extern PyObject *Py3dComponent_GetName(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored));
extern PyObject *Py3dComponent_SetName(struct Py3dComponent *self, PyObject *args, PyObject *kwds);
void Py3dComponent_SetNameCStr(struct Py3dComponent *self, const char *newName);
extern PyObject *Py3dComponent_GetOwner(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored));
extern PyObject *Py3dComponent_SetOwner(struct Py3dComponent *self, PyObject *args, PyObject *kwds);

extern bool Py3dComponent_CallParse(struct Py3dComponent *component, PyObject *data, struct ResourceManager *rm);

#endif
