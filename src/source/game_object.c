#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <stdlib.h>

#include "custom_string.h"
#include "logger.h"
#include "game_object.h"
#include "rendering_context.h"
#include "python/python_util.h"
#include "python/py3dcomponent.h"
#include "python/py3dtransform.h"

struct Py3dGameObject {
    PyObject_HEAD
    PyObject *componentsList;
    PyObject *childrenList;
    struct Py3dGameObject *parent;
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
    {"update", (PyCFunction) Py3dGameObject_Update, METH_VARARGS, "Update Game Object"},
    {"render", (PyCFunction) Py3dGameObject_Render, METH_VARARGS, "Render Game Object"},
    {"attach_child", (PyCFunction) Py3dGameObject_AttachChild, METH_VARARGS, "Attach a GameObject to another GameObject"},
    {"attach_component", (PyCFunction) Py3dGameObject_AttachComponent, METH_VARARGS, "Attach a Component to a GameObject"},
    {NULL}
};

PyTypeObject Py3dGameObject_Type = {
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
    if (PyType_Ready(&Py3dGameObject_Type) == -1) return false;

    if (PyModule_AddObject(module, "GameObject", (PyObject *) &Py3dGameObject_Type) == -1) return false;

    Py_INCREF(&Py3dGameObject_Type);

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
    return PyObject_IsInstance(obj, (PyObject *) &Py3dGameObject_Type);
}

static PyObject *getCallable(PyObject *obj, const char *callableName) {
    PyObject *callable = PyObject_GetAttrString(obj, callableName);
    if (callable == NULL) return NULL;
    if (!PyCallable_Check(callable)) {
        PyErr_Format(PyExc_TypeError, "[GameObject]: Object must have a callable attribute called \"%s\"", callableName);
        return NULL;
    }

    return callable;
}

static PyObject *passMessage(struct Py3dGameObject *self, const char *messageName, PyObject *args) {
    Py_ssize_t componentCount = PySequence_Size(self->componentsList);
    for (Py_ssize_t i = 0; i < componentCount; ++i) {
        PyObject *curComponent = PyList_GetItem(self->componentsList, i);
        if (!Py3dComponent_IsComponent(curComponent)) {
            warning_log("[GameObject]: Component list has non component item. Will not pass update message.");
            continue;
        }

        PyObject *messageHandler = getCallable((PyObject *) self, messageName);
        if (messageHandler == NULL) {
            warning_log("[GameObject]: Could not pass \"%s\" message to component", messageName);
            handleException();
            continue;
        }

        PyObject *ret = PyObject_Call(messageHandler, args, NULL);
        if (ret == NULL) {
            handleException();
        }
        Py_CLEAR(ret);
        Py_CLEAR(messageHandler);
    }

    Py_ssize_t childCount = PySequence_Size(self->childrenList);
    for (Py_ssize_t i = 0; i < childCount; ++i) {
        PyObject *curChild = PyList_GetItem(self->childrenList, i);
        if (!Py3dGameObject_Check(curChild)) {
            warning_log("[GameObject]: Child list has non Game Object child. Will not pass update message.");
            continue;
        }

        PyObject *messageHandler = getCallable((PyObject *) self, messageName);
        if (messageHandler == NULL) {
            warning_log("[GameObject]: Could not pass \"%s\" message to child", messageName);
            handleException();
            continue;
        }

        PyObject *ret = PyObject_Call(messageHandler, args, NULL);
        if (ret == NULL) {
            handleException();
        }

        Py_CLEAR(ret);
        Py_CLEAR(messageHandler);
    }

    Py_RETURN_NONE;
}

PyObject *Py3dGameObject_Update(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    float dt = 0.0f;
    if (PyArg_ParseTuple(args, "f", &dt) != 1) return NULL;

    return passMessage(self, "update", args);
}

PyObject *Py3dGameObject_Render(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    PyObject *renderingContext = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dRenderingContext_Type, &renderingContext) != 1) return NULL;

    return passMessage(self, "render", args);
}

PyObject *Py3dGameObject_AttachChild(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    PyObject *newChild = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dGameObject_Type, &newChild) != 1) return NULL;

    // TODO: figure out if I need to decref here
    if (PyList_Append(self->childrenList, newChild) != 0) {
        return NULL;
    }

    // TODO: this introduces a reference cycle and likely breaks garbage collection
    ((struct Py3dGameObject *) newChild)->parent = self;
    Py_RETURN_NONE;
}

PyObject *Py3dGameObject_AttachComponent(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    PyObject *newComponent = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dComponent_Type, &newComponent) != 1) return NULL;

    // TODO: figure out if I need to decref here
    if (PyList_Append(self->componentsList, newComponent) != 0) {
        return NULL;
    }

    // TODO: this introduces a reference cycle and likely breaks garbage collection
    ((struct Py3dComponent *) newComponent)->owner = self;
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
