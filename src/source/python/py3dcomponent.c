#include "python/py3dcomponent.h"

#include "custom_string.h"
#include "game_object.h"

#include "python/python_util.h"

static void Py3dComponent_Dealloc(struct Py3dComponent *self) {
    Py_CLEAR(self->name);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dComponent_Init(struct Py3dComponent *self, PyObject *args, PyObject *kwds) {
    Py_INCREF(Py_None);
    self->name = Py_None;
    self->owner = NULL;

    return 0;
}

// TODO: these don't seem to serve any purpose other than to be overridden by sub classes
// Maybe one day there will need to be code in these, although that should be avoided because it'll require
// subclasses to call "super" when overriding these methods
// for now lets just have these so I can hang a doc string on them describing how to override them
static PyObject *Py3dComponent_Update(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored)) {
    Py_RETURN_NONE;
}

static PyObject *Py3dComponent_Render(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored)) {
    Py_RETURN_NONE;
}

static PyObject *Py3dComponent_Parse(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored)) {
    Py_RETURN_NONE;
}

static PyObject *Py3dComponent_GetName(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored)) {
    if (self->name == NULL) {
        PyErr_SetString(PyExc_AttributeError, "name");
        return NULL;
    }

    Py_INCREF(self->name);
    return self->name;
}

PyObject *Py3dComponent_GetOwner(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored)) {
    if (self->owner == NULL) Py_RETURN_NONE;

    if (self->owner->pyGameObject == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Sanity check failed. Owner GameObject is detached from its python object");
        return NULL;
    }

    if (Py3dGameObject_Check((PyObject *) self->owner->pyGameObject) == 0) {
        PyErr_SetString(PyExc_AssertionError, "Sanity check failed. Owner is not a Game Object");
        return NULL;
    }

    Py_INCREF(self->owner->pyGameObject);
    return (PyObject *) self->owner->pyGameObject;
}

static PyMemberDef py3d_component_members[] = {
    {NULL}
};

static PyMethodDef py3d_component_methods[] = {
    {"update", (PyCFunction) Py3dComponent_Update, METH_VARARGS, "Update event handler"},
    {"render", (PyCFunction) Py3dComponent_Render, METH_VARARGS, "Render event handler"},
    {"get_name", (PyCFunction) Py3dComponent_GetName, METH_NOARGS, "Get component name"},
    {"get_owner", (PyCFunction) Py3dComponent_GetOwner, METH_NOARGS, "Get component owner"},
    {"parse", (PyCFunction) Py3dComponent_Parse, METH_VARARGS, "Parse json contents"},
    {NULL}
};

PyTypeObject Py3dComponent_Type = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "py3dengine.Component",
    .tp_basicsize = sizeof(struct Py3dComponent),
    .tp_dealloc = (destructor) Py3dComponent_Dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Base class for writing components",
    .tp_methods = py3d_component_methods,
    .tp_members = py3d_component_members,
    .tp_init = (initproc) Py3dComponent_Init,
    .tp_new = PyType_GenericNew
};

bool PyInit_Py3dComponent(PyObject *module) {
    if (PyType_Ready(&Py3dComponent_Type) < 0) return false;

    if (PyModule_AddObject(module, "Component", (PyObject *) &Py3dComponent_Type) < 0) return false;

    Py_INCREF(&Py3dComponent_Type);

    return true;
}

bool Py3dComponent_IsComponent(PyObject *pyObj) {
    if (pyObj == NULL) return false;

    int res = PyObject_IsInstance(pyObj, (PyObject *) &Py3dComponent_Type);
    if (res == -1) {
        handleException();
        return false;
    }

    return res == 1;
}