#ifndef PY3DENGINE_RESOURCE_MANAGER_H
#define PY3DENGINE_RESOURCE_MANAGER_H

#include <stdbool.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

struct BaseResource;
struct Py3dResourceManager;

struct ResourceManager {
    struct ListNode *_root;
    struct Py3dResourceManager *py3dResourceManager;
};

struct Py3dResourceManager {
    PyObject_HEAD
    struct ResourceManager *resourceManager;
};

extern PyTypeObject Py3dResourceManager_Type;

extern bool PyInit_Py3dResourceManager(PyObject *module);
extern bool findPy3dResourceManagerCtor(PyObject *module);
extern void finalizePy3dResourceManagerCtor();

extern int Py3dResourceManager_Check(PyObject *obj);

extern void allocResourceManager(struct ResourceManager **resourceManagerPtr);
extern void deleteResourceManager(struct ResourceManager **resourceManagerPtr);

extern void storeResource(struct ResourceManager *manager, struct BaseResource *resource);
extern struct BaseResource *getResource(struct ResourceManager *manager, const char *name);

#endif
