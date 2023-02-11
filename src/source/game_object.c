#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <stdlib.h>

#include "custom_string.h"
#include "logger.h"
#include "game_object.h"
#include "python/python_util.h"
#include "python/py3dcomponent.h"
#include "python/py3dtransform.h"

struct Py3dGameObject {
    PyObject_HEAD
    PyObject *componentsList;
    PyObject *childrenList;
    PyObject *parent;
    PyObject *name;
    PyObject *transform;
};

static PyObject *py3dGameObjectCtor = NULL;

static void Py3dGameObject_Dealloc(struct Py3dGameObject *self) {
    Py_CLEAR(self->transform);
    Py_CLEAR(self->name);
    Py_CLEAR(self->parent);
    Py_CLEAR(self->childrenList);
    Py_CLEAR(self->componentsList);

    Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dGameObject_Init(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    self->componentsList = PyList_New(0);
    self->childrenList = PyList_New(0);
    self->parent = Py_None;
    Py_INCREF(Py_None);
    self->name = Py_None;
    Py_INCREF(Py_None);
    self->transform = (PyObject *) Py3dTransform_New();

    return 0;
}

static PyObject *Py3dGameObject_GetName(struct Py3dGameObject *self, PyObject *Py_UNUSED(ignored)) {
    Py_INCREF(self->name);

    return self->name;
}

PyObject *Py3dGameObject_GetTransform(struct Py3dGameObject *self, PyObject *Py_UNUSED(ignored)) {
    Py_INCREF(self->transform);

    return (PyObject *) self->transform;
}

PyMethodDef Py3dGameObject_Methods[] = {
    {"get_name", (PyCFunction) Py3dGameObject_GetName, METH_NOARGS, "Get Game Object's name"},
    {"get_transform", (PyCFunction) Py3dGameObject_GetTransform, METH_NOARGS, "Get Game Object's transform"},
    {NULL}
};

PyTypeObject Py3dGameObjectType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "py3dengine.GameObject",
    .tp_basicsize = sizeof(struct Py3dGameObject),
    .tp_dealloc = (destructor) Py3dGameObject_Dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Class for manipulating Game Objects",
    .tp_methods = Py3dGameObject_Methods,
    .tp_init = (initproc) Py3dGameObject_Init,
    .tp_new = PyType_GenericNew
};

static PyObject *Py3dGameObject_New() {
    if (py3dGameObjectCtor == NULL) {
        critical_log("%s", "[Python]: Py3dGameObject has not been initialized properly");

        return NULL;
    }

    PyObject *py3dGameObject = PyObject_Call(py3dGameObjectCtor, PyTuple_New(0), NULL);
    if (py3dGameObject == NULL) {
        critical_log("%s", "[Python]: Failed to allocate GameObject in python interpreter");
        handleException();

        return NULL;
    }

    return py3dGameObject;
}

bool PyInit_Py3dGameObject(PyObject *module) {
    if (PyType_Ready(&Py3dGameObjectType) == -1) return false;

    if (PyModule_AddObject(module, "GameObject", (PyObject *) &Py3dGameObjectType) == -1) return false;

    Py_INCREF(&Py3dGameObjectType);

    return true;
}

bool Py3dGameObject_FindCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "GameObject") == 0) {
        critical_log("%s", "[Python]: Py3dGameObject has not been initialized properly");

        return false;
    }

    py3dGameObjectCtor = PyObject_GetAttrString(module, "GameObject");

    return true;
}

void Py3dGameObject_FinalizeCtor() {
    Py_CLEAR(py3dGameObjectCtor);
}

int Py3dGameObject_Check(PyObject *obj) {
    return PyObject_IsInstance(obj, (PyObject *) &Py3dGameObjectType);
}

static PyObject *Py3dGameObject_Update(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    float dt = 0.0f;
    if (PyArg_ParseTuple(args, "f", &dt) != 1) return NULL;

    Py_ssize_t componentCount = PySequence_Size(self->componentsList);
    for (Py_ssize_t i = 0; i < componentCount; ++i) {
        PyObject *curComponent = PyList_GetItem(self->componentsList, i);
        if (!Py3dComponent_IsComponent(curComponent)) {
            warning_log("[GameObject]: Component list has non component item. Will not pass update message.");
            continue;
        }

        Py3dComponent_CallUpdate((struct Py3dComponent *) curComponent, dt);
    }

    Py_ssize_t childCount = PySequence_Size(self->childrenList);
    for (Py_ssize_t i = 0; i < childCount; ++i) {
        PyObject *curChild = PyList_GetItem(self->childrenList, i);
        if (!Py3dGameObject_Check(curChild)) {
            warning_log("[GameObject]: Child list has non Game Object child. Will not pass update message.");
            continue;
        }

        PyObject *updateCallable = PyObject_GetAttrString(curChild, "Update");
        if
    }
}

void updateGameObject(struct GameObject *gameObject, float dt) {
    if (gameObject == NULL) return;

    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        Py3dComponent_CallUpdate(curNode->pyComponent, dt);

        curNode = curNode->next;
    }

    struct ChildListNode *curChild = gameObject->children;
    while (curChild != NULL) {
        updateGameObject(curChild->child, dt);

        curChild = curChild->next;
    }
}

void renderGameObject(struct GameObject *gameObject, struct RenderingContext *renderingContext) {
    if (gameObject == NULL || renderingContext == NULL) return;

    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        Py3dComponent_CallRender(curNode->pyComponent, renderingContext);

        curNode = curNode->next;
    }

    struct ChildListNode *curChild = gameObject->children;
    while (curChild != NULL) {
        renderGameObject(curChild->child, renderingContext);

        curChild = curChild->next;
    }
}

void attachChild(struct GameObject *parent, struct GameObject *newChild) {
    if (parent == NULL || newChild == NULL) return;

    struct String *newChildName = getGameObjectName(newChild);
    if (newChildName == NULL) {
        error_log("%s", "Cannot attach child game object unless it has a name");
        return;
    }

    struct ChildListNode *prevNode = NULL, *curNode = parent->children;
    while (curNode != NULL) {
        if (stringEquals(getGameObjectName(curNode->child), newChildName)) {
            error_log("%s", "Cannot attach child game object unless its name is unique");
            return;
        }

        prevNode = curNode;
        curNode = curNode->next;
    }

    struct ChildListNode *newNode = NULL;
    allocChildListNode(&newNode);
    if (newNode == NULL) return;
    newNode->child = newChild;

    if (prevNode == NULL) {
        parent->children = newNode;
    } else {
        prevNode->next = newNode;
    }
    newChild->parent = parent;
}

void removeChild(struct GameObject *gameObject, struct GameObject *target) {
    critical_log("%s", "GameObject::removeChild is not yet implemented");

    // TODO: implement GameObject::removeChild
}

void removeChildByName(struct GameObject *gameObject, const char* name) {
    critical_log("%s", "GameObject::removeChildByName is not yet implemented");

    // TODO: implement GameObject::removeChildByName
}

struct GameObject *findGameObjectByName(struct GameObject *gameObject, const char *name) {
    if (gameObject == NULL || name == NULL) return NULL;

    if (stringEqualsCStr(gameObject->name, name)) {
        return gameObject;
    }

    struct ChildListNode *curNode = gameObject->children;
    struct GameObject *ret = NULL;
    while (curNode != NULL) {
        ret = findGameObjectByName(curNode->child, name);
        if (ret != NULL) {
            return ret;
        }

        curNode = curNode->next;
    }

    return ret;
}

void attachPyComponent(struct GameObject *gameObject, struct Py3dComponent *newPyComponent) {
    if (gameObject == NULL || newPyComponent == NULL) return;

    PyObject *newComponentName = Py3dComponent_GetName(newPyComponent, NULL);
    if (newComponentName == NULL) {
        error_log("%s", "[GameObject]: Component threw exception while trying to get its name");
        handleException();
        return;
    }

    struct ComponentListNode *prevNode = NULL, *curNode = gameObject->components;
    while (curNode != NULL) {
        if (curNode->pyComponent == NULL) {
            critical_log("[GameObject]: Sanity check failure. NULL pyComponent found while iterating components list");
            continue;
        }

        PyObject *curComponentName = Py3dComponent_GetName(curNode->pyComponent, NULL);
        if (PyObject_RichCompareBool(newComponentName, curComponentName, Py_EQ) == 1) {
            error_log("%s", "[GameObject]: Component names must be unique. Rejecting attachment request");
            Py_CLEAR(newComponentName);
            Py_CLEAR(curComponentName);
            return;
        }
        Py_CLEAR(curComponentName);

        prevNode = curNode;
        curNode = curNode->next;
    }

    Py_CLEAR(newComponentName);

    struct ComponentListNode *newNode = NULL;
    allocComponentListNode(&newNode);
    if (newNode == NULL) return;

    newNode->pyComponent = newPyComponent;
    if (prevNode == NULL) {
        gameObject->components = newNode;
    } else {
        prevNode->next = newNode;
    }
    newPyComponent->owner = gameObject;
}

size_t getGameObjectComponentsLength(struct GameObject *gameObject) {
    if (gameObject == NULL) return -1;

    size_t count = 0;
    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        count++;
        curNode = curNode->next;
    }

    return count;
}

struct Py3dComponent *getGameObjectComponentByIndex(struct GameObject *gameObject, size_t index) {
    if (gameObject == NULL) return NULL;

    size_t count = 0;
    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        if (count == index) break;

        curNode = curNode->next;
    }

    if (curNode == NULL) return NULL;

    if (curNode->pyComponent == NULL) return NULL;

    return curNode->pyComponent;
}

struct String *getGameObjectName(struct GameObject *gameObject) {
    if (gameObject == NULL) return NULL;

    return gameObject->name;
}

void setGameObjectName(struct GameObject *gameObject, const char *newName) {
    if (gameObject == NULL) return;

    if (gameObject->name == NULL) {
        allocString(&(gameObject->name), newName);
    } else {
        setChars(gameObject->name, newName);
    }
}

struct Py3dTransform *getGameObjectTransform(struct GameObject *gameObject) {
    if (gameObject == NULL) return NULL;

    return gameObject->transform;
}
