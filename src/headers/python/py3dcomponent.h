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
    bool enabled;
    bool visible;
};

extern PyTypeObject Py3dComponent_Type;

extern int PyInit_Py3dComponent(PyObject *module);
extern int Py3dComponent_Check(PyObject *pyObj);
extern void Py3dComponent_Dealloc(struct Py3dComponent *self);
extern PyObject *Py3dComponent_IsEnabled(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored));
extern bool Py3dComponent_IsEnabledBool(struct Py3dComponent *self);
extern PyObject *Py3dComponent_Enable(struct Py3dComponent *self, PyObject *args, PyObject *kwds);
extern void Py3dComponent_EnableBool(struct Py3dComponent *self, bool enable);
extern PyObject *Py3dComponent_IsVisible(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored));
extern bool Py3dComponent_IsVisibleBool(struct Py3dComponent *self);
extern PyObject *Py3dComponent_MakeVisible(struct Py3dComponent *self, PyObject *args, PyObject *kwds);
extern void Py3dComponent_MakeVisibleBool(struct Py3dComponent *self, bool make_visible);
extern PyObject *Py3dComponent_GetName(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored));
extern PyObject *Py3dComponent_SetName(struct Py3dComponent *self, PyObject *args, PyObject *kwds);
extern void Py3dComponent_SetNameCStr(struct Py3dComponent *self, const char *newName);
extern PyObject *Py3dComponent_GetOwner(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored));
extern PyObject *Py3dComponent_SetOwner(struct Py3dComponent *self, PyObject *args, PyObject *kwds);

extern PyObject *Py3dComponent_Update(struct Py3dComponent *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dComponent_Parse(struct Py3dComponent *self, PyObject *args, PyObject *kwds);

#endif
