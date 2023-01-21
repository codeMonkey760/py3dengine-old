#include <stdlib.h>

#include "custom_string.h"
#include "resource_manager.h"
#include "logger.h"
#include "python/python_util.h"
#include "resources/base_resource.h"

struct ListNode {
    struct BaseResource *resource;

    struct ListNode *next;
};

static PyObject *Py3dResourceManager_Ctor = NULL;

static void Py3dResourceManager_Dealloc(struct Py3dResourceManager *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dResourceManager_Init(struct Py3dResourceManager *self, PyObject *args, PyObject *kwds) {
    self->resourceManager = NULL;

    return 0;
}

PyMethodDef Py3dResourceManager_Methods[] = {
    {NULL}
};

PyTypeObject Py3dResourceManager_Type = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "py3dengine.ResourceManager",
    .tp_basicsize = sizeof(struct Py3dResourceManager),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor) Py3dResourceManager_Dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "Class for storing imported resources",
    .tp_methods = Py3dResourceManager_Methods,
    .tp_init = (initproc) Py3dResourceManager_Init,
    .tp_new = PyType_GenericNew
};

bool PyInit_Py3dResourceManager(PyObject *module) {
    if (PyType_Ready(&Py3dResourceManager_Type) == -1) return false;

    if (PyModule_AddObject(module, "ResourceManager", (PyObject *) &Py3dResourceManager_Type) == -1) return false;

    Py_INCREF(&Py3dResourceManager_Type);

    return true;
}

bool findPy3dResourceManagerCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "ResourceManager") == 0) {
        critical_log("%s", "[Python]: Py3dResourceManager has not been initialized properly");

        return false;
    }

    Py3dResourceManager_Ctor = PyObject_GetAttrString(module, "ResourceManager");

    return true;
}

void finalizePy3dResourceManagerCtor() {
    Py_CLEAR(Py3dResourceManager_Ctor);
}

static struct Py3dResourceManager *Py3dResourceManager_New() {
    if (Py3dResourceManager_Ctor == NULL) {
        critical_log("%s", "[Python]: Py3dResourceManager has not been initialized properly");

        return NULL;
    }

    struct Py3dResourceManager *py3dResourceManager = (struct Py3dResourceManager *) PyObject_Call(Py3dResourceManager_Ctor, PyTuple_New(0), NULL);
    if (py3dResourceManager == NULL) {
        critical_log("%s", "[Python]: Failed to allocate ResourceManager in python interpreter");
        handleException();

        return NULL;
    }

    return py3dResourceManager;
}

static void allocListNode(struct ListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) != NULL) return;

    struct ListNode *newNode = calloc(1, sizeof(struct ListNode));
    newNode->resource = NULL;

    (*listNodePtr) = newNode;
    newNode = NULL;
}

static void deleteListNode(struct ListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) == NULL) return;

    struct ListNode *listNode = (*listNodePtr);
    deleteResource(&listNode->resource);

    deleteListNode(&listNode->next);

    free(listNode);
    listNode = NULL;
    (*listNodePtr) = NULL;
}

void allocResourceManager(struct ResourceManager **resourceManagerPtr){
    if (resourceManagerPtr == NULL || (*resourceManagerPtr) != NULL) return;

    struct ResourceManager *newResourceManager = calloc(1, sizeof(struct ResourceManager));
    if (newResourceManager == NULL) return;

    newResourceManager->_root = NULL;
    newResourceManager->py3dResourceManager = NULL;
    newResourceManager->py3dResourceManager = Py3dResourceManager_New();
    newResourceManager->py3dResourceManager->resourceManager = newResourceManager;

    (*resourceManagerPtr) = newResourceManager;
    newResourceManager = NULL;
}

void deleteResourceManager(struct ResourceManager **resourceManagerPtr){
    if (resourceManagerPtr == NULL || (*resourceManagerPtr) == NULL) return;

    struct ResourceManager *manager = (*resourceManagerPtr);

    deleteListNode(&manager->_root);
    Py_CLEAR(manager->py3dResourceManager);

    free(manager);
    manager = NULL;
    (*resourceManagerPtr) = NULL;
}

void storeResource(struct ResourceManager *manager, struct BaseResource *resource) {
    if (manager == NULL || resource == NULL) return;

    struct ListNode *prevNode = NULL, *curNode = manager->_root;
    while (curNode != NULL) {
        if (resourceNamesEqual(curNode->resource, resource)) {
            error_log(
                "[ResourceManager]: A name collision occurred while storing resource named \"%s\". The resource will likely be leaked.",
                getChars(getResourceName(resource))
            );
            return;
        }

        prevNode = curNode;
        curNode = curNode->next;
    }

    struct ListNode *newNode = NULL;
    allocListNode(&newNode);
    if (newNode == NULL) return;

    newNode->resource = resource;
    newNode->next = NULL;

    if (prevNode == NULL) {
        manager->_root = newNode;
    } else {
        prevNode->next = newNode;
    }
}

struct BaseResource *getResource(struct ResourceManager *manager, const char *name) {
    struct ListNode *curNode = manager->_root;
    while (curNode != NULL) {
        struct BaseResource *curResource = curNode->resource;
        if (curResource == NULL) {
            critical_log("%s", "[ResourceManager]: NULL pointer detected in resource list. Memory leak likely.");
            continue;
        }

        if (stringEqualsCStr(curResource->_name, name)) {
            return curNode->resource;
        }

        curNode = curNode->next;
    }

    return NULL;
}
