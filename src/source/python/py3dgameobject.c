#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "logger.h"
#include "python/py3dgameobject.h"
#include "python/python_util.h"
#include "python/py3dcomponent.h"
#include "python/py3dtransform.h"
#include "python/py3drenderingcontext.h"
#include "python/py3dcollisionevent.h"

struct Py3dGameObject {
    PyObject_HEAD
    bool enabled;
    bool visible;
    PyObject *componentsList;
    PyObject *childrenList;
    PyObject *parent;
    PyObject *name;
    PyObject *transform;
};

static PyObject *py3dGameObjectCtor = NULL;
static PyObject *getCallable(PyObject *obj, const char *callableName);
static PyObject *passMessage(struct Py3dGameObject *self, bool (*acceptMessage)(struct Py3dComponent *), const char *messageName, PyObject *args);

static int Py3dGameObject_Traverse(struct Py3dGameObject *self, visitproc visit, void *arg) {
    Py_VISIT(self->componentsList);
    Py_VISIT(self->childrenList);
    Py_VISIT(self->parent);
    Py_VISIT(self->transform);
    return 0;
}

static int Py3dGameObject_Clear(struct Py3dGameObject *self) {
    Py_CLEAR(self->transform);
    Py_CLEAR(self->name);
    Py_CLEAR(self->parent);
    Py_CLEAR(self->childrenList);
    Py_CLEAR(self->componentsList);
    return 0;
}

static void Py3dGameObject_Dealloc(struct Py3dGameObject *self) {
    trace_log("%s", "[GameObject]: Deallocating GameObject");

    Py3dGameObject_Clear(self);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dGameObject_Init(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    trace_log("%s", "[GameObject]: Initializing Game Object");

    self->enabled = true;
    self->visible = true;
    self->componentsList = PyList_New(0);
    self->childrenList = PyList_New(0);
    self->parent = Py_NewRef(Py_None);
    self->name = Py_NewRef(Py_None);
    self->transform = (PyObject *) Py3dTransform_New();

    return 0;
}

PyMethodDef Py3dGameObject_Methods[] = {
    {"enabled", (PyCFunction) Py3dGameObject_IsEnabled, METH_NOARGS, "Determine if a Game Object is enabled"},
    {"enable", (PyCFunction) Py3dGameObject_Enable, METH_VARARGS, "Enable or disable a Game Object"},
    {"visible", (PyCFunction) Py3dGameObject_IsVisible, METH_NOARGS, "Determine if a Game Object is visible"},
    {"make_visible", (PyCFunction) Py3dGameObject_MakeVisible, METH_VARARGS, "Make a Game Object visible or invisible"},
    {"get_name", (PyCFunction) Py3dGameObject_GetName, METH_NOARGS, "Get Game Object's name"},
    {"set_name", (PyCFunction) Py3dGameObject_SetName, METH_VARARGS, "Set Game Object's name"},
    {"get_transform", (PyCFunction) Py3dGameObject_GetTransform, METH_NOARGS, "Get Game Object's transform"},
    {"start", (PyCFunction) Py3dGameObject_Start, METH_VARARGS, "Propagate start message"},
    {"update", (PyCFunction) Py3dGameObject_Update, METH_VARARGS, "Update Game Object"},
    {"render", (PyCFunction) Py3dGameObject_Render, METH_VARARGS, "Render Game Object"},
    {"end", (PyCFunction) Py3dGameObject_End, METH_VARARGS, "Propagate end message"},
    {"attach_child", (PyCFunction) Py3dGameObject_AttachChild, METH_VARARGS, "Attach a GameObject to another GameObject"},
    {"get_child_by_name", (PyCFunction) Py3dGameObject_GetChildByName, METH_VARARGS, "Get a ref to the first child with the specified name"},
    {"get_child_by_index", (PyCFunction) Py3dGameObject_GetChildByIndex, METH_VARARGS, "Get a ref to the child at the specified index"},
    {"get_child_count", (PyCFunction) Py3dGameObject_GetChildCount, METH_NOARGS, "Get the number of children this GameObject has"},
    {"attach_component", (PyCFunction) Py3dGameObject_AttachComponent, METH_VARARGS, "Attach a Component to a GameObject"},
    {"get_component_by_type", (PyCFunction) Py3dGameObject_GetComponentByType, METH_VARARGS, "Get a ref to the first Component of the specified type"},
    {"get_component_by_index", (PyCFunction) Py3dGameObject_GetComponentByIndex, METH_VARARGS, "Get a ref to the component at the specified index"},
    {"get_component_count", (PyCFunction) Py3dGameObject_GetComponentCount, METH_NOARGS, "Get the number of components this GameObject has"},
    {NULL}
};

PyTypeObject Py3dGameObject_Type = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "py3dengine.GameObject",
    .tp_basicsize = sizeof(struct Py3dGameObject),
    .tp_dealloc = (destructor) Py3dGameObject_Dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,
    .tp_doc = "Class for manipulating Game Objects",
    .tp_methods = Py3dGameObject_Methods,
    .tp_init = (initproc) Py3dGameObject_Init,
    .tp_new = PyType_GenericNew,
    .tp_traverse = (traverseproc) Py3dGameObject_Traverse,
    .tp_clear = (inquiry) Py3dGameObject_Clear
};

int PyInit_Py3dGameObject(PyObject *module) {
    if (PyType_Ready(&Py3dGameObject_Type) == -1) return 0;

    if (PyModule_AddObject(module, "GameObject", (PyObject *) &Py3dGameObject_Type) == -1) return 0;

    Py_INCREF(&Py3dGameObject_Type);

    return 1;
}

int Py3dGameObject_FindCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "GameObject") == 0) {
        critical_log("%s", "[Python]: Py3dGameObject has not been initialized properly");

        return 0;
    }

    py3dGameObjectCtor = PyObject_GetAttrString(module, "GameObject");

    return 1;
}

void Py3dGameObject_FinalizeCtor() {
    Py_CLEAR(py3dGameObjectCtor);
}

int Py3dGameObject_Check(PyObject *obj) {
    int ret = PyObject_IsInstance(obj, (PyObject *) &Py3dGameObject_Type);
    if (ret == -1) {
        handleException();
        return 0;
    }

    return ret;
}

PyObject *Py3dGameObject_New() {
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

PyObject *Py3dGameObject_IsEnabled(struct Py3dGameObject *self, PyObject *Py_UNUSED(ignored)) {
    return PyBool_FromLong(Py3dGameObject_IsEnabledBool(self));
}

bool Py3dGameObject_IsEnabledBool(struct Py3dGameObject *self) {
    return self->enabled;
}

PyObject *Py3dGameObject_Enable(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    PyObject *enableObj = NULL;
    if (PyArg_ParseTuple(args, "O!", &PyBool_Type, &enableObj) != 1) return NULL;

    bool enable = Py_IsTrue(enableObj);
    Py3dGameObject_EnableBool(self, enable);

    Py_RETURN_NONE;
}

void Py3dGameObject_EnableBool(struct Py3dGameObject *self, bool enable) {
    self->enabled = enable;
}

PyObject *Py3dGameObject_IsVisible(struct Py3dGameObject *self, PyObject *Py_UNUSED(ignored)) {
    return PyBool_FromLong(Py3dGameObject_IsVisibleBool(self));
}

bool Py3dGameObject_IsVisibleBool(struct Py3dGameObject *self) {
    return self->visible;
}

PyObject *Py3dGameObject_MakeVisible(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    PyObject *makeVisibleObj = NULL;
    if (PyArg_ParseTuple(args, "O!", &PyBool_Type, &makeVisibleObj) != 1) return NULL;

    bool make_visible = Py_IsTrue(makeVisibleObj);
    Py3dGameObject_MakeVisibleBool(self, make_visible);

    Py_RETURN_NONE;
}

void Py3dGameObject_MakeVisibleBool(struct Py3dGameObject *self, bool make_visible) {
    self->visible = make_visible;
}

PyObject *Py3dGameObject_GetName(struct Py3dGameObject *self, PyObject *Py_UNUSED(ignored)) {
    PyObject *ret = self->name;

    Py_INCREF(ret);
    return ret;
}

const char *Py3dGameObject_GetNameCStr(struct Py3dGameObject *self) {
    if (Py_IsNone(self->name)) {
        return NULL;
    }

    return PyUnicode_AsUTF8(self->name);
}

extern PyObject *Py3dGameObject_SetName(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    PyObject *newName = NULL;
    if (PyArg_ParseTuple(args, "O!", &PyUnicode_Type, &newName) != 1) return NULL;

    Py_DECREF(self->name);
    Py_INCREF(newName);
    self->name = newName;

    Py_RETURN_NONE;
}

extern void Py3dGameObject_SetNameCStr(struct Py3dGameObject *self, const char *newName) {
    if (newName == NULL) return;

    PyObject *newNameObj = PyUnicode_FromString(newName);

    Py_DECREF(self->name);
    self->name = newNameObj;
}

PyObject *Py3dGameObject_GetTransform(struct Py3dGameObject *self, PyObject *Py_UNUSED(ignored)) {
    Py_INCREF(self->transform);

    return (PyObject *) self->transform;
}

PyObject *Py3dGameObject_Start(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    return passMessage(self, NULL, "start", args);
}

PyObject *Py3dGameObject_Update(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    if (self->enabled == false) Py_RETURN_NONE;

    float dt = 0.0f;
    if (PyArg_ParseTuple(args, "f", &dt) != 1) return NULL;

    return passMessage(self, Py3dComponent_IsEnabledBool, "update", args);
}

PyObject *Py3dGameObject_Render(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    if (self->visible == false) Py_RETURN_NONE;

    PyObject *renderingContext = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dRenderingContext_Type, &renderingContext) != 1) return NULL;

    return passMessage(self, Py3dComponent_IsVisibleBool, "render", args);
}

PyObject *Py3dGameObject_End(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    return passMessage(self, NULL, "end", args);
}

void Py3dGameObject_Collide(struct Py3dGameObject *self, struct Py3dCollisionEvent *event) {
    if (self->enabled == false) return;

    PyObject *args = PyTuple_New(1);
    PyTuple_SetItem(args, 1, (PyObject *) event);

    PyObject *ret = passMessage(self, Py3dComponent_IsEnabledBool, "collide", args);
    if (ret == NULL) {
        handleException();
    }

    Py_CLEAR(ret);
    Py_CLEAR(args);
}

extern void Py3dGameObject_ColliderEnter(struct Py3dGameObject *self, struct Py3dCollisionEvent *event) {
    if (self->enabled == false) return;

    PyObject *args = Py_BuildValue("(O)", event);

    PyObject *ret = passMessage(self, Py3dComponent_IsEnabledBool, "collider_enter", args);
    if (ret == NULL) {
        handleException();
    }

    Py_CLEAR(ret);
    Py_CLEAR(args);
}

extern void Py3dGameObject_ColliderExit(struct Py3dGameObject *self, struct Py3dCollisionEvent *event) {
    if (self->enabled == false) return;

    PyObject *args = Py_BuildValue("(O)", event);

    PyObject *ret = passMessage(self, Py3dComponent_IsEnabledBool, "collider_exit", args);
    if (ret == NULL) {
        handleException();
    }

    Py_CLEAR(ret);
    Py_CLEAR(args);
}

PyObject *Py3dGameObject_AttachChild(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    PyObject *newChild = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dGameObject_Type, &newChild) != 1) return NULL;

    if (PyList_Append(self->childrenList, newChild) != 0) {
        return NULL;
    }

    ((struct Py3dGameObject *) newChild)->parent = (PyObject *) self;
    Py_INCREF(self);

    Py_RETURN_NONE;
}

static PyObject *do_GetChildByName(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    PyObject *name = NULL;
    if (PyArg_ParseTuple(args, "O!", &PyUnicode_Type, &name) != 1) return NULL;

    PyObject *ret = Py_None;

    Py_ssize_t childCount = PySequence_Size(self->childrenList);
    for (Py_ssize_t i = 0; i < childCount; ++i) {
        PyObject *curChild = PyList_GetItem(self->childrenList, i);
        if (Py3dGameObject_Check(curChild) != 1) {
            warning_log("%s", "[GameObject]: Child list contains non GameObject entry");
            continue;
        }
        PyObject *curChildName = Py3dGameObject_GetName((struct Py3dGameObject *) curChild, NULL);
        int cmpResult = PyObject_RichCompareBool(name, curChildName, Py_EQ);
        if (cmpResult == -1) {
            handleException();
        } else if (cmpResult == 1) {
            ret = curChild;
        }

        Py_CLEAR(curChildName);
    }

    if (!Py_IsNone(ret)) return ret;

    for (Py_ssize_t i = 0; i < childCount; ++i) {
        PyObject *curChild = PyList_GetItem(self->childrenList, i);
        if (Py3dGameObject_Check(curChild) != 1) {
            warning_log("%s", "[GameObject]: Child list contains non GameObject entry");
            continue;
        }

        if (Py_EnterRecursiveCall(" in Py3dGameObject_GetChildByName") != 0) return NULL;
        ret = Py3dGameObject_GetChildByName((struct Py3dGameObject *) curChild, args, kwds);
        Py_LeaveRecursiveCall();

        if (ret == NULL) return NULL;
        if (!Py_IsNone(ret)) break;
    }

    return ret;
}

PyObject *Py3dGameObject_GetChildByName(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    PyObject *ret = do_GetChildByName(self, args, kwds);
    Py_XINCREF(ret);

    return ret;
}

PyObject *Py3dGameObject_GetChildByNameCStr(struct Py3dGameObject *self, const char *name) {
    PyObject *args = Py_BuildValue("(s)", name);

    PyObject *ret = Py3dGameObject_GetChildByName(self, args, NULL);

    Py_CLEAR(args);
    return ret;
}

PyObject *Py3dGameObject_GetChildByIndex(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    PyObject *indexAsObj = NULL;
    if (PyArg_ParseTuple(args, "O", &indexAsObj) != 1) return NULL;

    Py_ssize_t index;
    index = PyNumber_AsSsize_t(indexAsObj, PyExc_IndexError);
    if (index == -1) return NULL;

    PyObject *ret = Py3dGameObject_GetChildByIndexInt(self, index);
    if (ret == NULL) return NULL;

    Py_INCREF(ret);
    return ret;
}

PyObject *Py3dGameObject_GetChildByIndexInt(struct Py3dGameObject *self, Py_ssize_t index) {
    return PyList_GetItem(self->childrenList, index);
}

PyObject *Py3dGameObject_GetChildCount(struct Py3dGameObject *self, PyObject *Py_UNUSED(ignored)) {
    return PyLong_FromSsize_t(Py3dGameObject_GetChildCountInt(self));
}

Py_ssize_t Py3dGameObject_GetChildCountInt(struct Py3dGameObject *self) {
    return PySequence_Size(self->childrenList);
}

PyObject *Py3dGameObject_AttachComponent(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    PyObject *newComponent = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dComponent_Type, &newComponent) != 1) return NULL;

    if (PyList_Append(self->componentsList, newComponent) != 0) {
        return NULL;
    }

    ((struct Py3dComponent *) newComponent)->owner = (PyObject *) self;
    Py_INCREF(self);

    Py_RETURN_NONE;
}

PyObject *Py3dGameObject_GetComponentByType(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    PyObject *typeObj = NULL;
    if (PyArg_ParseTuple(args, "O!", &PyType_Type, &typeObj) != 1) return NULL;

    PyObject *ret = Py_None;

    Py_ssize_t componentCount = PySequence_Size(self->componentsList);
    for (Py_ssize_t i = 0; i < componentCount; ++i) {
        PyObject *curComponent = PyList_GetItem(self->componentsList, i);
        int isInstance = PyObject_IsInstance(curComponent, typeObj);
        if (isInstance == -1) {
            handleException();
        } else if (isInstance == 1) {
            ret = curComponent;
            break;
        }
    }

    Py_INCREF(ret);
    return ret;
}

PyObject *Py3dGameObject_GetComponentByIndex(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    PyObject *indexAsObj = NULL;
    if (PyArg_ParseTuple(args, "O", &indexAsObj) == -1) return NULL;

    Py_ssize_t index;
    index = PyNumber_AsSsize_t(indexAsObj, PyExc_IndexError);
    if (index == -1) return NULL;

   return Py3dGameObject_GetChildByIndexInt(self, index);
}

PyObject *Py3dGameObject_GetComponentByIndexInt(struct Py3dGameObject *self, Py_ssize_t index) {
    PyObject *ret = PyList_GetItem(self->componentsList, index);

    Py_XINCREF(ret);
    return ret;
}

PyObject *Py3dGameObject_GetComponentCount(struct Py3dGameObject *self, PyObject *Py_UNUSED(ignored)) {
    return PyLong_FromSsize_t(Py3dGameObject_GetChildCountInt(self));
}

Py_ssize_t Py3dGameObject_GetComponentCountInt(struct Py3dGameObject *self) {
    return PySequence_Size(self->componentsList);
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

static void passMessageToComponent(PyObject *component, bool(*acceptMessage)(struct Py3dComponent *), const char *messageName, PyObject *args) {
    if (!Py3dComponent_Check(component)) {
        warning_log("[GameObject]: GameObject list has non component item.");
        return;
    }

    if (acceptMessage != NULL) {
        if (!acceptMessage((struct Py3dComponent *) component)) return;
    }

    PyObject *messageHandler = getCallable((PyObject *) component, messageName);
    if (messageHandler == NULL) {
        PyErr_Clear();
        return;
    }

    PyObject *ret = PyObject_Call(messageHandler, args, NULL);
    if (ret == NULL) {
        handleException();
    }
    Py_CLEAR(ret);
    Py_CLEAR(messageHandler);
}

static PyObject *passMessage(struct Py3dGameObject *self, bool (*acceptMessage)(struct Py3dComponent *), const char *messageName, PyObject *args) {
    PyObject *transform = Py3dGameObject_GetTransform(self, NULL);
    passMessageToComponent(transform, acceptMessage, messageName, args);
    Py_CLEAR(transform);

    Py_ssize_t componentCount = PySequence_Size(self->componentsList);
    for (Py_ssize_t i = 0; i < componentCount; ++i) {
        PyObject *curComponent = Py_NewRef(PyList_GetItem(self->componentsList, i));
        passMessageToComponent(curComponent, acceptMessage, messageName, args);
        Py_CLEAR(curComponent);
    }

    Py_ssize_t childCount = PySequence_Size(self->childrenList);
    for (Py_ssize_t i = 0; i < childCount; ++i) {
        PyObject *curChild = PyList_GetItem(self->childrenList, i);
        if (!Py3dGameObject_Check(curChild)) {
            warning_log("[GameObject]: Child list has non Game Object child. Will not pass update message.");
            continue;
        }

        PyObject *messageHandler = getCallable((PyObject *) curChild, messageName);
        if (messageHandler == NULL) {
            warning_log("[GameObject]: Could not pass \"%s\" message to child", messageName);
            handleException();
            continue;
        }

        if (Py_EnterRecursiveCall(" in GameObject::passMessage") != 0) {
            critical_log("[GameObject]: Hit max recursion depth while passing message \"%s\" to children", messageName);
            handleException();
            Py_CLEAR(messageHandler);
            continue;
        }
        PyObject *ret = PyObject_Call(messageHandler, args, NULL);
        if (ret == NULL) {
            handleException();
        }
        Py_LeaveRecursiveCall();

        Py_CLEAR(ret);
        Py_CLEAR(messageHandler);
    }

    Py_RETURN_NONE;
}