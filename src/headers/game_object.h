#ifndef PY3DENGINE_GAME_OBJECT_H
#define PY3DENGINE_GAME_OBJECT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

struct Py3dGameObject;
extern PyTypeObject Py3dGameObject_Type;

extern int PyInit_Py3dGameObject(PyObject *module);
extern int Py3dGameObject_FindCtor(PyObject *module);
extern void Py3dGameObject_FinalizeCtor();
extern int Py3dGameObject_Check(PyObject *obj);
extern PyObject *Py3dGameObject_New();
extern PyObject *Py3dGameObject_GetName(struct Py3dGameObject *self, PyObject *Py_UNUSED(ignored));
extern const char *Py3dGameObject_GetNameCStr(struct Py3dGameObject *self);
extern PyObject *Py3dGameObject_SetName(struct Py3dGameObject *self, PyObject *args, PyObject *kwds);
extern void Py3dGameObject_SetNameCStr(struct Py3dGameObject *self, const char *newName);
extern PyObject *Py3dGameObject_GetTransform(struct Py3dGameObject *self, PyObject *Py_UNUSED(ignored));
extern PyObject *Py3dGameObject_Update(struct Py3dGameObject *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dGameObject_Render(struct Py3dGameObject *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dGameObject_AttachChild(struct Py3dGameObject *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dGameObject_GetChildByName(struct Py3dGameObject *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dGameObject_GetChildByIndex(struct Py3dGameObject *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dGameObject_GetChildByIndexInt(struct Py3dGameObject *self, Py_ssize_t index);
extern PyObject *Py3dGameObject_GetChildCount(struct Py3dGameObject *self, PyObject *Py_UNUSED(ignored));
extern Py_ssize_t Py3dGameObject_GetChildCountInt(struct Py3dGameObject *self);
extern PyObject *Py3dGameObject_AttachComponent(struct Py3dGameObject *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dGameObject_GetComponentByType(struct Py3dGameObject *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dGameObject_GetComponentByIndex(struct Py3dGameObject *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dGameObject_GetComponentByIndexInt(struct Py3dGameObject *self, Py_ssize_t index);
extern PyObject *Py3dGameObject_GetComponentCount(struct Py3dGameObject *self, PyObject *Py_UNUSED(ignored));
extern Py_ssize_t Py3dGameObject_GetComponentCountInt(struct Py3dGameObject *self);

#endif
