#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "logger.h"
#include "python/py3dgameobject.h"
#include "python/python_util.h"
#include "python/py3dcomponent.h"
#include "python/py3drenderingcontext.h"
#include "python/py3dcollisionevent.h"
#include "python/py3dscene.h"
#include "math/vector3.h"
#include "math/quaternion.h"
#include "util.h"

struct Py3dGameObject {
    PyObject_HEAD
    bool enabled;
    bool visible;
    struct Py3dScene *scene;
    PyObject *componentsList;
    PyObject *childrenList;
    PyObject *parent;
    PyObject *name;
    // TODO: these will be defined in world space until matrix chaining has been implemented
    // Then they will be defined relative to the parent game object's space
    PyObject *position;
    PyObject *orientation;
    PyObject *scale;
    int matrixCacheDirty;
    float wMatrixCache[16];
    float witMatrixCache[16];
};

static PyObject *py3dGameObjectCtor = NULL;
static PyObject *getCallable(PyObject *obj, const char *callableName);
static PyObject *passMessage(struct Py3dGameObject *self, int (*acceptMessage)(struct Py3dComponent *), const char *messageName, PyObject *args);

static int Py3dGameObject_Traverse(struct Py3dGameObject *self, visitproc visit, void *arg) {
    Py_VISIT(self->componentsList);
    Py_VISIT(self->childrenList);
    Py_VISIT(self->parent);
    Py_VISIT(self->scene);
    return 0;
}

static int Py3dGameObject_Clear(struct Py3dGameObject *self) {
    Py_CLEAR(self->scale);
    Py_CLEAR(self->orientation);
    Py_CLEAR(self->position);
    Py_CLEAR(self->name);
    Py_CLEAR(self->parent);
    Py_CLEAR(self->childrenList);
    Py_CLEAR(self->componentsList);
    Py_CLEAR(self->scene);
    return 0;
}

static void Py3dGameObject_Dealloc(struct Py3dGameObject *self) {
    trace_log("%s", "[GameObject]: Deallocating GameObject");

    Py3dGameObject_Clear(self);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dGameObject_Init(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    trace_log("%s", "[GameObject]: Initializing Game Object");

    struct Py3dScene *newScene = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dScene_Type, &newScene) != 1) {
        return -1;
    }

    self->enabled = true;
    self->visible = true;
    self->componentsList = PyList_New(0);
    self->childrenList = PyList_New(0);
    self->parent = Py_NewRef(Py_None);
    self->name = Py_NewRef(Py_None);
    self->scene = (struct Py3dScene *) Py_NewRef(newScene);
    self->position = (PyObject *) Py3dVector3_New(0.0f, 0.0f, 0.0f);
    self->orientation = (PyObject *) Py3dQuaternion_New(0.0f, 0.0f, 0.0f, 1.0f);
    self->scale = (PyObject *) Py3dVector3_New(1.0f, 1.0f, 1.0f);
    self->matrixCacheDirty = 0;
    Mat4Identity(self->wMatrixCache);
    Mat4Identity(self->witMatrixCache);

    return 0;
}

static PyObject *Py3dGameObject_GetScenePython(struct Py3dGameObject *self, PyObject *Py_UNUSED(ignored)) {
    return Py_NewRef(self->scene);
}

PyMethodDef Py3dGameObject_Methods[] = {
    {"enabled", (PyCFunction) Py3dGameObject_IsEnabled, METH_NOARGS, "Determine if a Game Object is enabled"},
    {"enable", (PyCFunction) Py3dGameObject_Enable, METH_VARARGS, "Enable or disable a Game Object"},
    {"visible", (PyCFunction) Py3dGameObject_IsVisible, METH_NOARGS, "Determine if a Game Object is visible"},
    {"make_visible", (PyCFunction) Py3dGameObject_MakeVisible, METH_VARARGS, "Make a Game Object visible or invisible"},
    {"get_scene", (PyCFunction) Py3dGameObject_GetScenePython, METH_NOARGS, "Get Game Object's scene"},
    {"get_name", (PyCFunction) Py3dGameObject_GetName, METH_NOARGS, "Get Game Object's name"},
    {"set_name", (PyCFunction) Py3dGameObject_SetName, METH_VARARGS, "Set Game Object's name"},
    {"start", (PyCFunction) Py3dGameObject_Start, METH_VARARGS, "Propagate start message"},
    {"activate", (PyCFunction) Py3dGameObject_Activate, METH_VARARGS, "Propagate activate message"},
    {"update", (PyCFunction) Py3dGameObject_Update, METH_VARARGS, "Update Game Object"},
    {"render", (PyCFunction) Py3dGameObject_Render, METH_VARARGS, "Render Game Object"},
    {"deactivate", (PyCFunction) Py3dGameObject_Deactivate, METH_VARARGS, "Propagate deactivate message"},
    {"end", (PyCFunction) Py3dGameObject_End, METH_VARARGS, "Propagate end message"},
    {"attach_child", (PyCFunction) Py3dGameObject_AttachChild, METH_VARARGS, "Attach a GameObject to another GameObject"},
    {"get_child_by_name", (PyCFunction) Py3dGameObject_GetChildByName, METH_VARARGS, "Get a ref to the first child with the specified name"},
    {"get_child_by_index", (PyCFunction) Py3dGameObject_GetChildByIndex, METH_VARARGS, "Get a ref to the child at the specified index"},
    {"get_child_count", (PyCFunction) Py3dGameObject_GetChildCount, METH_NOARGS, "Get the number of children this GameObject has"},
    {"attach_component", (PyCFunction) Py3dGameObject_AttachComponent, METH_VARARGS, "Attach a Component to a GameObject"},
    {"detach_component", (PyCFunction) Py3dGameObject_DetachComponent, METH_VARARGS, "Detach a Component from a GameObject"},
    {"get_component_by_type", (PyCFunction) Py3dGameObject_GetComponentByType, METH_VARARGS, "Get a ref to the first Component of the specified type"},
    {"get_component_by_index", (PyCFunction) Py3dGameObject_GetComponentByIndex, METH_VARARGS, "Get a ref to the component at the specified index"},
    {"get_component_count", (PyCFunction) Py3dGameObject_GetComponentCount, METH_NOARGS, "Get the number of components this GameObject has"},
    {"get_position", (PyCFunction) Py3dGameObject_GetPosition, METH_NOARGS, "Get position"},
    {"move", (PyCFunction) Py3dGameObject_Move, METH_VARARGS, "Move the game object by displacement value"},
    {"set_position", (PyCFunction) Py3dGameObject_SetPosition, METH_VARARGS, "Set the position to absolute value"},
    {"get_orientation", (PyCFunction) Py3dGameObject_GetOrientation, METH_NOARGS, "Get orientation"},
    {"rotate", (PyCFunction) Py3dGameObject_Rotate, METH_VARARGS, "Rotate the game object by displacement value"},
    {"set_orientation", (PyCFunction) Py3dGameObject_SetOrientation, METH_VARARGS, "Set the orientation to absolute value"},
    {"get_scale", (PyCFunction) Py3dGameObject_GetScale, METH_NOARGS, "Get scale"},
    {"stretch", (PyCFunction) Py3dGameObject_Stretch, METH_VARARGS, "Stretch the transform by factor value"},
    {"set_scale", (PyCFunction) Py3dGameObject_SetScale, METH_VARARGS, "Set the scale to absolute value"},
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
    if (obj == NULL) return 0;

    int ret = PyObject_IsInstance(obj, (PyObject *) &Py3dGameObject_Type);
    if (ret == -1) {
        handleException();
        return 0;
    }

    return ret;
}

struct Py3dGameObject *Py3dGameObject_New(struct Py3dScene *newScene) {
    if (py3dGameObjectCtor == NULL) {
        critical_log("%s", "[Python]: Py3dGameObject has not been initialized properly");

        return NULL;
    }

    if (newScene == NULL) {
        error_log("[GameObject]: Could not instantiate. Requires valid scene pointer.");

        return NULL;
    }

    PyObject *args = Py_BuildValue("(O)", newScene);
    PyObject *ret = PyObject_Call(py3dGameObjectCtor, args, NULL);
    Py_CLEAR(args);
    if (ret == NULL || !Py3dGameObject_Check(ret)) {
        Py_CLEAR(ret);
        critical_log("%s", "[Python]: Failed to allocate GameObject in python interpreter");
        handleException();

        return NULL;
    }

    return (struct Py3dGameObject *) ret;
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

PyObject *Py3dGameObject_Start(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    return passMessage(self, NULL, "start", args);
}

PyObject *Py3dGameObject_Activate(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    return passMessage(self, NULL, "activate", args);
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

PyObject *Py3dGameObject_Deactivate(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    return passMessage(self, NULL, "deactivate", args);
}

PyObject *Py3dGameObject_End(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    return passMessage(self, NULL, "end", args);
}

void Py3dGameObject_Collide(struct Py3dGameObject *self, struct Py3dCollisionEvent *event) {
    if (self->enabled == false) return;

    PyObject *args = PyTuple_New(1);
    PyTuple_SetItem(args, 1, (PyObject *) event);

    // TODO: this is a bug ... this message is propagated to children of this GO
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

    // TODO: this is a bug ... this message is propagated to children of this GO
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

    // TODO: this is a bug ... this message is propagated to children of this GO
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

void Py3dGameObject_AttachComponentInC(struct Py3dGameObject *self, struct Py3dComponent *component) {
    if (Py3dGameObject_Check((PyObject *) self) != 1 || Py3dComponent_Check((PyObject *) component) != 1) return;

    // TODO: I think that PyList_Append is incrementing the ref count of component ... double check
    if (PyList_Append(self->componentsList, (PyObject *) component) != 0) {
        handleException();
        return;
    }

    Py_CLEAR(component->owner);
    component->owner = Py_NewRef((PyObject *) self);
}

PyObject *Py3dGameObject_AttachComponent(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    struct Py3dComponent *newComponent = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dComponent_Type, &newComponent) != 1) return NULL;

    Py3dGameObject_AttachComponentInC(self, newComponent);

    Py_RETURN_NONE;
}

void Py3dGameObject_DetachComponentInC(struct Py3dGameObject *self, struct Py3dComponent *component) {
    if (Py3dGameObject_Check((PyObject *) self) != 1 || Py3dComponent_Check((PyObject *) component) != 1) return;

    PyObject *methodName = PyUnicode_FromString("remove");
    PyObject *ret = PyObject_CallMethodOneArg(self->componentsList, methodName, (PyObject *) component);
    Py_CLEAR(methodName);
    if (ret == NULL) {
        handleException();
    }
    Py_CLEAR(ret);
}

extern PyObject *Py3dGameObject_DetachComponent(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    struct Py3dComponent *target = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dComponent_Type, &target) != 1) return NULL;

    Py3dGameObject_DetachComponentInC(self, target);

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

   return Py3dGameObject_GetComponentByIndexInt(self, index);
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

struct Py3dScene *Py3dGameObject_GetScene(struct Py3dGameObject *self) {
    return (struct Py3dScene *) Py_NewRef(self->scene);
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

static void passMessageToComponent(PyObject *component, int(*acceptMessage)(struct Py3dComponent *), const char *messageName, PyObject *args) {
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

static PyObject *passMessage(struct Py3dGameObject *self, int (*acceptMessage)(struct Py3dComponent *), const char *messageName, PyObject *args) {
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

const float *Py3dGameObject_GetPositionFA(struct Py3dGameObject *self) {
    return ((struct Py3dVector3 *) self->position)->elements;
}

PyObject *Py3dGameObject_GetPosition(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    return Py_NewRef(self->position);
}

PyObject *Py3dGameObject_Move(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    struct Py3dVector3 *displacement = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dVector3_Type, &displacement) != 1) return NULL;

    PyObject *result = PyNumber_Add((PyObject *) self->position, (PyObject *) displacement);
    if (result == NULL) return NULL;

    Py_CLEAR(self->position);
    self->position = result; // New Ref acquired from PyNumber_Add call

    self->matrixCacheDirty = 1;

    Py_RETURN_NONE;
}

PyObject *Py3dGameObject_SetPosition(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    struct Py3dVector3 *newPosition = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dVector3_Type, &newPosition) != 1) return NULL;

    Py_CLEAR(self->position);
    self->position = Py_NewRef(newPosition);

    self->matrixCacheDirty = 1;

    Py_RETURN_NONE;
}

const float *Py3dGameObject_GetOrientationFA(struct Py3dGameObject *self) {
    return ((struct Py3dQuaternion *) self->orientation)->elements;
}

PyObject *Py3dGameObject_GetOrientation(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    return Py_NewRef(self->orientation);
}
PyObject *Py3dGameObject_Rotate(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    struct Py3dQuaternion *displacement = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dQuaternion_Type, &displacement) != 1) return NULL;

    PyObject *result = PyNumber_Multiply((PyObject *) self->orientation,(PyObject *) displacement);
    if (result == NULL) return NULL;

    Py_CLEAR(self->orientation);
    self->orientation = result; // New Ref acquired from PyNumber_Multiply call

    self->matrixCacheDirty = 1;

    Py_RETURN_NONE;
}

PyObject *Py3dGameObject_SetOrientation(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    struct Py3dQuaternion *newOrientation = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dQuaternion_Type, &newOrientation) != 1) return NULL;

    Py_CLEAR(self->orientation);
    self->orientation = Py_NewRef(newOrientation);

    self->matrixCacheDirty = 1;

    Py_RETURN_NONE;
}

const float *Py3dGameObject_GetScaleFA(struct Py3dGameObject *self) {
    return ((struct Py3dVector3 *) self->scale)->elements;
}

PyObject *Py3dGameObject_GetScale(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    return Py_NewRef(self->orientation);
}
PyObject *Py3dGameObject_Stretch(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    struct Py3dVector3 *factor = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dVector3_Type, &factor) != 1) return NULL;

    PyObject *result = PyNumber_Multiply((PyObject *) self->orientation,(PyObject *) factor);
    if (result == NULL) return NULL;

    Py_CLEAR(self->scale);
    self->scale = result; // New Ref acquired from PyNumber_Multiply call

    self->matrixCacheDirty = 1;

    Py_RETURN_NONE;
}

PyObject *Py3dGameObject_SetScale(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    struct Py3dVector3 *newScale = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dVector3_Type, &newScale) != 1) return NULL;

    Py_CLEAR(self->scale);
    self->scale = Py_NewRef(newScale);

    self->matrixCacheDirty = 1;

    Py_RETURN_NONE;
}

static void refreshMatrixCaches(struct Py3dGameObject *self) {
    if (self->matrixCacheDirty == 0) return;

    // TODO: all of this nasty matrix multiplication can be removed for a substantial optimization
    // work out the needed component multiplication on paper so that we can remove all of the
    // multiplying by 1 and 0
    float sMtx[16] = {0.0f};

    Mat4ScalingFA(sMtx, Py3dGameObject_GetScaleFA(self));

    float rMtx[16] = {0.0f};
    Mat4RotationQuaternionFA(rMtx, Py3dGameObject_GetOrientationFA(self));

    float tMtx[16] = {0.0f};
    Mat4TranslationFA(tMtx, Py3dGameObject_GetPositionFA(self));

    float wMtx[16] = {0.0f};
    Mat4Mult(wMtx, sMtx, rMtx);
    Mat4Mult(wMtx, wMtx, tMtx);

    Mat4Copy(self->wMatrixCache, wMtx);

    Mat4Inverse(self->witMatrixCache, wMtx);
    Mat4Transpose(self->witMatrixCache, self->witMatrixCache);

    self->matrixCacheDirty = 0;
}

const float *Py3dGameObject_GetWorldMatrix(struct Py3dGameObject *self) {
    refreshMatrixCaches(self);

    return self->wMatrixCache;
}

const float *Py3dGameObject_GetWITMatrix(struct Py3dGameObject *self) {
    refreshMatrixCaches(self);

    return self->witMatrixCache;
}

void Py3dGameObject_CalculateViewMatrix(struct Py3dGameObject *self, float dst[16]) {
    if (dst == NULL) return;

    float camTarget[3] = {0.0f, 0.0f, 1.0f};
    float camUp[3] = {0.0f, 1.0f, 0.0f};

    const float *pos = Py3dGameObject_GetPositionFA(self);
    const float *orientation = Py3dGameObject_GetOrientationFA(self);

    // TODO: this can probably be optimized as well
    QuaternionVec3Rotation(camTarget, orientation, camTarget);
    Vec3Add(camTarget, pos, camTarget);

    QuaternionVec3Rotation(camUp, orientation, camUp);

    Mat4LookAtLH(dst, pos, camTarget, camUp);
}

