#ifndef PY3DENGINE_PY3DCOLLISIONEVENT_H
#define PY3DENGINE_PY3DCOLLISIONEVENT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

struct Py3dCollisionEvent {
    PyObject_HEAD
    PyObject *rigidBody1;
    PyObject *rigidBody2;
    PyObject *contactsTuple;
};

extern PyTypeObject Py3dCollisionEvent_Type;

extern int PyInit_Py3dCollisionEvent(PyObject *module);
extern int Py3dCollisionEvent_FindCtor(PyObject *module);
extern void Py3dCollisionEvent_FinalizeCtor();
extern struct Py3dCollisionEvent *Py3dCollisionEvent_New();
extern int Py3dCollisionEvent_Check(PyObject *obj);

extern PyObject *Py3dCollisionEvent_GetRigidBody1(struct Py3dCollisionEvent *self, void *closure);
extern PyObject *Py3dCollisionEvent_GetRigidBody2(struct Py3dCollisionEvent *self, void *closure);
extern PyObject *Py3dCollisionEvent_GetContacts(struct Py3dCollisionEvent *self, void *closure);

#endif
