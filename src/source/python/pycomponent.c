#include "python/pycomponent.h"

#include "custom_string.h"
#include "game_object.h"
#include "logger.h"
#include "python/python_util.h"

static PyObject *py3dComponentCtor = NULL;

static void py3d_component_dealloc(struct Py3dComponent *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *py3d_component_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    struct Py3dComponent *self = (struct Py3dComponent *) type->tp_alloc(type, 0);
    if (self == NULL) return NULL;

    self->name = PyUnicode_FromString("");
    if (self->name == NULL) {
        Py_DECREF(self);
        return NULL;
    }

    self->owner = NULL;

    return (PyObject *) self;
}

static int py3d_component_init(struct Py3dComponent *self, PyObject *args, PyObject *kwds) {
    return 0;
}

// TODO: these don't seem to serve any purpose other than to be overridden by sub classes
// Maybe one day there will need to be code in these, although that should be avoided because it'll require
// subclasses to call "super" when overriding these methods
// for now lets just have these so I can hang a doc string on them describing how to override them
static PyObject *py3d_component_update(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored)) {
    Py_RETURN_NONE;
}

static PyObject *py3d_component_render(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored)) {
    Py_RETURN_NONE;
}

static PyObject *py3d_component_get_name(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored)) {
    if (self->name == NULL) {
        PyErr_SetString(PyExc_AttributeError, "name");
        return NULL;
    }

    return self->name;
}

static PyObject *py3d_component_get_owner(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored)) {
    if (self->owner == NULL) Py_RETURN_NONE;

    if (self->owner->pyGameObject == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Sanity check failed. Owner GameObject is detached from its python object");
        return NULL;
    }

    return (PyObject *) self->owner->pyGameObject;
}

static PyMemberDef py3d_component_members[] = {
    {NULL}
};

static PyMethodDef py3d_component_methods[] = {
    {"update", (PyCFunction) py3d_component_update, METH_VARARGS, "Update event handler"},
    {"render", (PyCFunction) py3d_component_render, METH_VARARGS, "Render event handler"},
    {"get_name", (PyCFunction) py3d_component_get_name, METH_NOARGS, "Get component name"},
    {"get_owner", (PyCFunction) py3d_component_get_owner, METH_NOARGS, "Get component owner"},
    {NULL}
};

static PyTypeObject Py3dComponentType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "py3dengine.Component",
    .tp_basicsize = sizeof(struct Py3dComponent),
    .tp_dealloc = (destructor) py3d_component_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Base class for writing components",
    .tp_methods = py3d_component_methods,
    .tp_members = py3d_component_members,
    .tp_init = (initproc) py3d_component_init,
    .tp_new = py3d_component_new
};

bool PyInit_Py3dComponent(PyObject *module) {
    if (PyType_Ready(&Py3dComponentType) < 0) return false;

    if (PyModule_AddObject(module, "Component", (PyObject *) &Py3dComponentType) < 0) return false;

    Py_INCREF(&Py3dComponentType);

    return true;
}

bool findPyComponentCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "Component") == 0) {
        critical_log("%s", "[Python]: Py3dComponent has not been initialized properly");

        return false;
    }

    py3dComponentCtor = PyObject_GetAttrString(module, "Component");

    return true;
}

PyObject *createPyComponent() {
    if (py3dComponentCtor == NULL) {
        critical_log("%s", "[Python]: Py3dComponent has not been initialized properly");

        return NULL;
    }

    PyObject *py3dComponent = PyObject_Call(py3dComponentCtor, PyTuple_New(0), NULL);
    if (py3dComponent == NULL) {
        critical_log("%s", "[Python]: Failed to allocate Component in python interpreter");
        handleException();

        return NULL;
    }

    return py3dComponent;
}

void finalizePyComponentCtor() {
    Py_CLEAR(py3dComponentCtor);
}
