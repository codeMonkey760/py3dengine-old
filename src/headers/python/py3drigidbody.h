#ifndef PY3DENGINE_PY3DRIGIDBODY_H
#define PY3DENGINE_PY3DRIGIDBODY_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <ode/ode.h>

#include "py3dcomponent.h"

struct Py3dRigidBody;
extern PyTypeObject Py3dRigidBody_Type;

extern int PyInit_Py3dRigidBody(PyObject *module);
extern struct Py3dRigidBody *Py3dRigidBody_New();
extern int Py3dRigidBody_Check(PyObject *obj);

extern PyObject *Py3dRigidBody_SetShape(struct Py3dRigidBody *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dRigidBody_Parse(struct Py3dRigidBody *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dRigidBody_Update(struct Py3dRigidBody *self, PyObject *args, PyObject *kwds);
extern int Py3dRigidBody_IsTriggerInt(struct Py3dRigidBody *self);
extern PyObject *Py3dRigidBody_IsTrigger(struct Py3dRigidBody *self, PyObject *args, PyObject *kwds);
extern void Py3dRigidBody_MakeTriggerInt(struct Py3dRigidBody *self, int is_trigger);
extern PyObject *Py3dRigidBody_MakeTrigger(struct Py3dRigidBody *self, PyObject *args, PyObject *kwds);

#endif
