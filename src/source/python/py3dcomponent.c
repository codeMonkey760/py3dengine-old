#include "python/py3dcomponent.h"

#include "custom_string.h"
#include "game_object.h"

#include "python/python_util.h"
#include "logger.h"
#include "rendering_context.h"
#include "resource_manager.h"

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

PyObject *Py3dComponent_GetName(struct Py3dComponent *self, PyObject *Py_UNUSED(ignored)) {
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

void Py3dComponent_CallUpdate(struct Py3dComponent *component, float dt) {
    if (component == NULL) return;

    PyObject *pyUpdate = PyObject_GetAttrString((PyObject *) component, "update");
    if (pyUpdate == NULL) {
        handleException();
        return;
    }

    PyObject *componentName = Py3dComponent_GetName(component, NULL);
    const char *componentNameAsCStr = NULL;
    if (componentName == NULL) {
        handleException();
        componentNameAsCStr = "NAME_ERROR";
    } else {
        componentNameAsCStr = PyUnicode_AsUTF8(componentName);
    }

    PyObject *dtArg = PyFloat_FromDouble(dt);
    PyObject *pyUpdateRet = PyObject_CallOneArg(pyUpdate, dtArg);
    if (pyUpdateRet == NULL) {
        error_log("[Py3dComponent]: Python component named \"%s\" threw exception while updating", componentNameAsCStr);
        handleException();
    } else if (!Py_IsNone(pyUpdateRet)) {
        warning_log("%s", "[Py3dComponent]: Python component named \"%s\" returned something while updating, which is weird", componentNameAsCStr);
    }

    Py_CLEAR(componentName);
    Py_CLEAR(dtArg);
    Py_CLEAR(pyUpdateRet);
    Py_CLEAR(pyUpdate);
}

void Py3dComponent_CallRender(struct Py3dComponent *component, struct RenderingContext *renderingContext) {
    if (component == NULL || renderingContext == NULL) return;

    PyObject *pyRender = PyObject_GetAttrString((PyObject *) component, "render");
    if (pyRender == NULL) {
        handleException();
        return;
    }

    PyObject *componentName = Py3dComponent_GetName(component, NULL);
    const char *componentNameAsCStr = NULL;
    if (componentName == NULL) {
        handleException();
        componentNameAsCStr = "NAME_ERROR";
    } else {
        componentNameAsCStr = PyUnicode_AsUTF8(componentName);
    }

    PyObject *py3dRenderingContext = (PyObject *) renderingContext->py3dRenderingContext;
    if (py3dRenderingContext == NULL) {
        critical_log("[Py3dComponent]: Python component named \"%s\" failed sanity check. Its render function was passed a mal-formed rendering context");
        Py_CLEAR(pyRender);
        Py_CLEAR(componentName);
    }

    PyObject *pyRenderRet = PyObject_CallOneArg(pyRender, py3dRenderingContext);
    if (pyRenderRet == NULL) {
        error_log("[Py3dComponent]: Python component named \"%s\" threw exception while rendering", componentNameAsCStr);
        handleException();
    } else if (!Py_IsNone(pyRenderRet)) {
        warning_log("%s", "[Py3dComponent]: Python component named \"%s\" returned something while rendering, which is weird", componentNameAsCStr);
    }

    Py_CLEAR(componentName);
    Py_CLEAR(pyRenderRet);
    Py_CLEAR(pyRender);
}

bool Py3dComponent_CallParse(struct Py3dComponent *component, PyObject *data, struct ResourceManager *rm) {
    if (component == NULL || data == NULL) return false;

    PyObject *pyParse = PyObject_GetAttrString((PyObject *) component, "parse");
    if (pyParse == NULL) {
        handleException();

        return false;
    }

    PyObject *parseArgs = Py_BuildValue("(OO)", data, rm->py3dResourceManager);
    PyObject *parseRet = PyObject_Call(pyParse, parseArgs, NULL);
    if (parseRet == NULL) {
        handleException();
        Py_CLEAR(parseArgs);
        Py_CLEAR(pyParse);

        return false;
    }

    Py_CLEAR(parseRet);
    Py_CLEAR(parseArgs);
    Py_CLEAR(pyParse);

    return true;
}