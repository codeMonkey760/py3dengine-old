#ifndef PY3DENGINE_PY3DRESOURCEMANAGER_H
#define PY3DENGINE_PY3DRESOURCEMANAGER_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

struct Py3dScene;
struct ListNode;
struct BaseResource;

struct Py3dResourceManager {
    PyObject_HEAD
    struct Py3dScene *owner;

    struct ListNode *_root;
};

extern PyTypeObject Py3dResourceManager_Type;

extern int PyInit_Py3dResourceManager(PyObject *module);
extern int findPy3dResourceManagerCtor(PyObject *module);
extern void finalizePy3dResourceManagerCtor();

extern int Py3dResourceManager_Check(PyObject *obj);
struct Py3dResourceManager *Py3dResourceManager_New();

extern PyObject *Py3dResourceManager_GetOwner(struct Py3dResourceManager *self, PyObject *Py_UNUSED(ignored));
extern void Py3dResourceManager_SetOwnerInC(struct Py3dResourceManager *self, struct Py3dScene *newOwner);

extern void Py3dResourceManager_StoreResource(struct Py3dResourceManager *self, struct BaseResource *resource);
extern struct BaseResource *Py3dResourceManager_GetResource(struct Py3dResourceManager *self, const char *name);

#endif
