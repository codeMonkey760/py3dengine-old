#include "python/py3dresourcemanager.h"
#include "python/python_util.h"
#include "logger.h"
#include "custom_string.h"
#include "resources/base_resource.h"
#include "python/py3dscene.h"

struct ListNode {
    struct BaseResource *resource;

    struct ListNode *next;
};

static PyObject *Py3dResourceManager_Ctor = NULL;

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

static int Py3dResourceManager_Traverse(struct Py3dResourceManager *self, visitproc visit, void *arg) {
    Py_VISIT(self->owner);
    return 0;
}

static int Py3dResourceManager_Clear(struct Py3dResourceManager *self) {
    Py_CLEAR(self->owner);
    deleteListNode(&self->_root);
    return 0;
}

static void Py3dResourceManager_Dealloc(struct Py3dResourceManager *self) {
    Py3dResourceManager_Clear(self);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dResourceManager_Init(struct Py3dResourceManager *self, PyObject *args, PyObject *kwds) {
    self->_root = NULL;
    self->owner = NULL;

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
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,
    .tp_doc = "Class for storing imported resources",
    .tp_methods = Py3dResourceManager_Methods,
    .tp_init = (initproc) Py3dResourceManager_Init,
    .tp_new = PyType_GenericNew,
    .tp_traverse = (traverseproc) Py3dResourceManager_Traverse,
    .tp_clear = (inquiry) Py3dResourceManager_Clear
};

int PyInit_Py3dResourceManager(PyObject *module) {
    if (PyType_Ready(&Py3dResourceManager_Type) == -1) return 0;

    if (PyModule_AddObject(module, "ResourceManager", (PyObject *) &Py3dResourceManager_Type) == -1) return 0;

    Py_INCREF(&Py3dResourceManager_Type);

    return 1;
}

int findPy3dResourceManagerCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "ResourceManager") == 0) {
        critical_log("%s", "[Python]: Py3dResourceManager has not been initialized properly");

        return 0;
    }

    Py3dResourceManager_Ctor = PyObject_GetAttrString(module, "ResourceManager");

    return 1;
}

void finalizePy3dResourceManagerCtor() {
    Py_CLEAR(Py3dResourceManager_Ctor);
}

int Py3dResourceManager_Check(PyObject *obj) {
    if (obj == NULL) return 0;

    int ret = PyObject_IsInstance(obj, (PyObject *) &Py3dResourceManager_Type);
    if (ret == -1) {
        handleException();
        return 0;
    }

    return ret;
}

struct Py3dResourceManager *Py3dResourceManager_New() {
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

extern PyObject *Py3dResourceManager_GetOwner(struct Py3dResourceManager *self, PyObject *Py_UNUSED(ignored)) {
    if (self->owner == NULL) {
        Py_RETURN_NONE;
    } else {
        return Py_NewRef(self->owner);
    }
}

extern void Py3dResourceManager_SetOwnerInC(struct Py3dResourceManager *self, struct Py3dScene *newOwner) {
    if (Py3dResourceManager_Check((PyObject *) self) != 1 || Py3dScene_Check((PyObject *) newOwner) != 1) return;

    Py_CLEAR(self->owner);
    self->owner = (struct Py3dScene *) Py_NewRef(newOwner);
}

void Py3dResourceManager_StoreResource(struct Py3dResourceManager *self, struct BaseResource *resource) {
    if (Py3dResourceManager_Check((PyObject *) self) != 1 || resource == NULL) return;

    struct ListNode *prevNode = NULL, *curNode = self->_root;
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
        self->_root = newNode;
    } else {
        prevNode->next = newNode;
    }
}

struct BaseResource *Py3dResourceManager_GetResource(struct Py3dResourceManager *self, const char *name) {
    if (Py3dResourceManager_Check((PyObject *) self) != 1 || name == NULL) return NULL;

    struct ListNode *curNode = self->_root;
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
