#ifndef COMPONENT_HELPER_H
#define COMPONENT_HELPER_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

extern int Py3d_InitComponentHelper();
extern void Py3d_FinalizeComponentHelper();
extern PyTypeObject *Py3d_GetComponentType();
extern struct Py3dGameObject *Py3d_GetComponentOwner(PyObject *component);
extern struct Py3dGameObject *Py3d_GetOwnerForComponent(PyObject *component);
extern struct Py3dScene *Py3d_GetSceneForComponent(PyObject *component);
extern int Py3d_IsComponentSubclass(PyObject *obj);
extern PyObject *Py3d_CallSuperMethod(PyObject *obj, const char* name, PyObject *args, PyObject *kwds);
extern int Py3d_CallSuperInit(PyObject *obj, PyObject *args, PyObject *kwds);
extern int Py3d_IsComponentEnabled(PyObject *obj);
extern int Py3d_IsComponentVisible(PyObject *obj);

#endif
