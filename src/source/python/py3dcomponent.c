#include "python/py3dcomponent.h"

#include <structmember.h>

#include "logger.h"
#include "custom_string.h"
#include "python/python_util.h"
#include "python/py3dresourcemanager.h"

static int Py3dComponent_Traverse(struct Py3dComponent *self, visitproc visit, void *arg);
static int Py3dComponent_Clear(struct Py3dComponent *self);
static int Py3dComponent_Init(struct Py3dComponent *self, PyObject *args, PyObject *kwds);
static PyObject *Py3dComponent_Start(struct Py3dComponent *self, PyObject *args, PyObject *kwds);
static PyObject *Py3dComponent_Render(struct Py3dComponent *self, PyObject *args, PyObject *kwds);
static PyObject *Py3dComponent_End(struct Py3dComponent *self, PyObject *args, PyObject *kwds);
static PyObject *Py3dComponent_Collide(struct Py3dComponent *self, PyObject *args, PyObject *kwds);
static PyObject *Py3dComponent_ColliderEnter(struct Py3dComponent *self, PyObject *args, PyObject *kwds);
static PyObject *Py3dComponent_ColliderExit(struct Py3dComponent *self, PyObject *args, PyObject *kwds);

static PyMemberDef py3d_component_members[] = { {NULL} };
static PyMethodDef py3d_component_methods[] = {
        {"enabled", (PyCFunction) Py3dComponent_IsEnabled, METH_NOARGS, "Determine if a Component is enabled"},
        {"enable", (PyCFunction) Py3dComponent_Enable, METH_VARARGS, "Enable or disable a Component"},
        {"visible", (PyCFunction) Py3dComponent_IsVisible, METH_NOARGS, "Determine if a Component is visible"},
        {"make_visible", (PyCFunction) Py3dComponent_MakeVisible, METH_VARARGS, "Make a Component visible or invisible"},
        {"start", (PyCFunction) Py3dComponent_Start, METH_VARARGS, "Start event handler"},
        {"update", (PyCFunction) Py3dComponent_Update, METH_VARARGS, "Update event handler"},
        {"render", (PyCFunction) Py3dComponent_Render, METH_VARARGS, "Render event handler"},
        {"end", (PyCFunction) Py3dComponent_End, METH_VARARGS, "End event handler"},
        {"collide", (PyCFunction) Py3dComponent_Collide, METH_VARARGS, "Handle per tick collision events"},
        {"collider_enter", (PyCFunction) Py3dComponent_ColliderEnter, METH_VARARGS, "Handle collision enter events"},
        {"collider_exit", (PyCFunction) Py3dComponent_ColliderExit, METH_VARARGS, "Handle collision exit events"},
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
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,
    .tp_doc = "Base class for writing components",
    .tp_methods = py3d_component_methods,
    .tp_members = py3d_component_members,
    .tp_init = (initproc) Py3dComponent_Init,
    .tp_new = PyType_GenericNew,
    .tp_traverse = (traverseproc) Py3dComponent_Traverse,
    .tp_clear = (inquiry) Py3dComponent_Clear
};

int PyInit_Py3dComponent(PyObject *module) {
    if (PyType_Ready(&Py3dComponent_Type) < 0) return 0;

    if (PyModule_AddObject(module, "Component", (PyObject *) &Py3dComponent_Type) < 0) return 0;

    Py_INCREF(&Py3dComponent_Type);

    return 1;
}

int Py3dComponent_Check(PyObject *pyObj) {
    if (pyObj == NULL) return 0;

    int res = PyObject_IsInstance(pyObj, (PyObject *) &Py3dComponent_Type);
    if (res == -1) {
        handleException();
        return 0;
    }

    return res;
}

PyObject *Py3dComponent_IsEnabled(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored)) {
    return PyBool_FromLong(Py3dComponent_IsEnabledBool(self));
}

int Py3dComponent_IsEnabledBool(struct Py3dComponent *self) {
    return self->enabled;
}

PyObject *Py3dComponent_Enable(struct Py3dComponent *self, PyObject *args, PyObject *kwds) {
    PyObject *enableObj = NULL;
    if (PyArg_ParseTuple(args, "O!", &PyBool_Type, &enableObj) != 1) return NULL;

    int enable = Py_IsTrue(enableObj);
    Py3dComponent_EnableBool(self, enable);

    Py_RETURN_NONE;
}

void Py3dComponent_EnableBool(struct Py3dComponent *self, int enable) {
    self->enabled = enable;
}

PyObject *Py3dComponent_IsVisible(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored)) {
    return PyBool_FromLong(Py3dComponent_IsVisibleBool(self));
}

int Py3dComponent_IsVisibleBool(struct Py3dComponent *self) {
    return self->visible;
}

PyObject *Py3dComponent_MakeVisible(struct Py3dComponent *self, PyObject *args, PyObject *kwds) {
    PyObject *makeVisibleObj = NULL;
    if (PyArg_ParseTuple(args, "O!", &PyBool_Type, &makeVisibleObj) != 1) return NULL;

    int make_visible = Py_IsTrue(makeVisibleObj);
    Py3dComponent_MakeVisibleBool(self, make_visible);

    Py_RETURN_NONE;
}

void Py3dComponent_MakeVisibleBool(struct Py3dComponent *self, int make_visible) {
    self->visible = make_visible;
}

PyObject *Py3dComponent_GetName(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored)) {
    Py_INCREF(self->name);
    return self->name;
}

PyObject *Py3dComponent_SetName(struct Py3dComponent *self, PyObject *args, PyObject *kwds) {
    PyObject *newNameObj = NULL;
    if (PyArg_ParseTuple(args, "O!", &PyUnicode_Type, &newNameObj) != 1) return NULL;

    PyObject *oldName = self->name;
    self->name = newNameObj;
    Py_CLEAR(oldName);

    Py_RETURN_NONE;
}

void Py3dComponent_SetNameCStr(struct Py3dComponent *self, const char *newName) {
    PyObject *newNameObject = PyUnicode_FromString(newName);
    if (newNameObject == NULL) {
        handleException();
        return;
    }

    PyObject *setNameArgs = Py_BuildValue("(O)", newNameObject);
    PyObject *setNameRet = Py3dComponent_SetName(self, setNameArgs, NULL);
    if (setNameRet == NULL) {
        Py_CLEAR(setNameArgs);
        Py_CLEAR(newNameObject);
        handleException();
        return;
    }

    Py_CLEAR(setNameRet);
    Py_CLEAR(setNameArgs);
}

PyObject *Py3dComponent_GetOwner(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored)) {
    Py_INCREF(self->owner);
    return self->owner;
}

static int Py3dComponent_Traverse(struct Py3dComponent *self, visitproc visit, void *arg) {
    Py_VISIT(self->owner);

    return 0;
}

static int Py3dComponent_Clear(struct Py3dComponent *self) {
    Py_CLEAR(self->name);
    Py_CLEAR(self->owner);

    return 0;
}

static int Py3dComponent_Init(struct Py3dComponent *self, PyObject *args, PyObject *kwds) {
    trace_log("[Component]: Initializing Component of type \"%s\"", Py_TYPE(self)->tp_name);

    self->name = Py_NewRef(Py_None);
    self->owner = Py_NewRef(Py_None);
    self->enabled = true;
    self->visible = true;

    return 0;
}

void Py3dComponent_Dealloc(struct Py3dComponent *self) {
    trace_log("[Component]: Deallocating Component of type \"%s\"", Py_TYPE(self)->tp_name);

    Py3dComponent_Clear(self);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

// TODO: these don't seem to serve any purpose other than to be overridden by sub classes
// Maybe one day there will need to be code in these, although that should be avoided because it'll require
// subclasses to call "super" when overriding these methods
// for now lets just have these so I can hang a doc string on them describing how to override them
static PyObject *Py3dComponent_Start(struct Py3dComponent *self, PyObject *args, PyObject *kwds) {
    Py_RETURN_NONE;
}

PyObject *Py3dComponent_Update(struct Py3dComponent *self, PyObject *args, PyObject *kwds) {
    Py_RETURN_NONE;
}

static PyObject *Py3dComponent_Render(struct Py3dComponent *self, PyObject *args, PyObject *kwds) {
    Py_RETURN_NONE;
}

static PyObject *Py3dComponent_End(struct Py3dComponent *self, PyObject *args, PyObject *kwds) {
    Py_RETURN_NONE;
}

static PyObject *Py3dComponent_Collide(struct Py3dComponent *self, PyObject *args, PyObject *kwds) {
    Py_RETURN_NONE;
}

static PyObject *Py3dComponent_ColliderEnter(struct Py3dComponent *self, PyObject *args, PyObject *kwds) {
    Py_RETURN_NONE;
}

static PyObject *Py3dComponent_ColliderExit(struct Py3dComponent *self, PyObject *args, PyObject *kwds) {
    Py_RETURN_NONE;
}

PyObject *Py3dComponent_Parse(struct Py3dComponent *self, PyObject *args, PyObject *kwds) {
    PyObject *parseDataDict = NULL;
    struct Py3dResourceManager *rm = NULL;
    if (PyArg_ParseTuple(args, "O!O!", &PyDict_Type, &parseDataDict, &Py3dResourceManager_Type, &rm) != 1) return NULL;

    PyObject *nameObj = PyDict_GetItemString(parseDataDict, "name");
    if (nameObj == NULL || PyUnicode_Check(nameObj) != 1) {
        PyErr_SetString(PyExc_ValueError, "Component must have a string called \"name\"");
        return NULL;
    }
    Py_CLEAR(self->name);
    self->name = Py_NewRef(nameObj);

    PyObject *enabledObj = PyDict_GetItemString(parseDataDict, "enabled");
    if (enabledObj != NULL && PyBool_Check(enabledObj) == 1) {
        // TODO: warn if set but not valid?
        self->enabled = Py_IsTrue(enabledObj);
    }

    PyObject *visibleObj = PyDict_GetItemString(parseDataDict, "visible");
    if (visibleObj != NULL && PyBool_Check(visibleObj) == 1) {
        // TODO: warn if set but not valid?
        self->visible = Py_IsTrue(visibleObj);
    }

    Py_RETURN_NONE;
}

