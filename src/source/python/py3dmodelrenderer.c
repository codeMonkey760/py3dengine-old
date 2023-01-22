#include "python/py3dmodelrenderer.h"
#include "logger.h"
#include "python/python_util.h"
#include "python/py3dtransform.h"
#include "game_object.h"

static PyObject *Py3dModelRenderer_Ctor = NULL;

static struct Py3dTransform *getTransform(struct Py3dModelRenderer *self) {
    PyObject *owner = Py3dComponent_GetOwner((struct Py3dComponent *) self, NULL);
    if (owner == NULL) {
        return NULL;
    } else if (Py_IsNone(owner)) {
        Py_CLEAR(owner);
        PyErr_SetString(PyExc_ValueError, "Cannot render a component that is detached from scene graph");
        return NULL;
    }

    PyObject *transform = Py3dGameObject_GetTransform((struct Py3dGameObject *) owner, NULL);
    Py_CLEAR(owner);
    if (transform == NULL) {
        return NULL;
    } else if (Py_IsNone(transform)) {
        Py_CLEAR(transform);
        PyErr_SetString(PyExc_ValueError, "Cannot render a component who's parent does not have a transform");
        return NULL;
    }

    return (struct Py3dTransform *) transform;
}

static PyObject *Py3dModelRenderer_Render(struct Py3dModelRenderer *self, PyObject *args, PyObject *kwds) {
    if (self->shader == NULL || self->model == NULL || self->material == NULL) {
        PyErr_SetString(PyExc_ValueError, "ModelRendererComponent is not correctly configured");
        return NULL;
    }

    struct Py3dTransform *transform = getTransform(self);
    if (transform == NULL) return NULL;



    Py_RETURN_NONE;
}

static PyMethodDef Py3dModelRenderer_Methods[] = {
    {"render", (PyCFunction) Py3dModelRenderer_Render, METH_VARARGS, "Render function for ModelRendererComponent"},
    {NULL}
};

static void Py3dModelRenderer_Dealloc(struct Py3dModelRenderer *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dModelRenderer_Init(struct Py3dModelRenderer *self, PyObject *args, PyObject *kwds) {
    if (Py3dComponent_Type.tp_init((PyObject *) self, args, kwds) == -1) return -1;

    self->material = NULL;
    self->model = NULL;
    self->shader = NULL;

    return 0;
}

static PyTypeObject Py3dModelRenderer_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py3dengine.ModelRendererComponent",
    .tp_doc = "Renders a 3d model with a shader and a material",
    .tp_basicsize = sizeof(struct Py3dModelRenderer),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) Py3dModelRenderer_Init,
    .tp_methods = Py3dModelRenderer_Methods,
    .tp_dealloc = (destructor) Py3dModelRenderer_Dealloc,
    .tp_new = PyType_GenericNew
};

bool PyInit_Py3dModelRenderer(PyObject *module) {
    Py3dModelRenderer_Type.tp_base = &Py3dComponent_Type;
    if (PyType_Ready(&Py3dModelRenderer_Type) < 0) return false;

    if (PyModule_AddObject(module, "ModelRendererComponent", (PyObject *) &Py3dModelRenderer_Type) < 0) return false;

    Py_INCREF(&Py3dModelRenderer_Type);

    return true;
}

bool Py3dModelRenderer_FindCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "ModelRendererComponent") == 0) {
        critical_log("%s", "[Python]: Py3dModelRenderer has not been initialized properly");

        return false;
    }

    Py3dModelRenderer_Ctor = PyObject_GetAttrString(module, "ModelRendererComponent");

    return true;
}

void Py3dModelRenderer_FinalizeCtor() {
    Py_CLEAR(Py3dModelRenderer_Ctor);
}

struct Py3dModelRenderer *Py3dModelRenderer_New() {
    if (Py3dModelRenderer_Ctor == NULL) {
        critical_log("%s", "[Python]: Py3dModelRenderer has not been initialized properly");

        return NULL;
    }

    struct Py3dModelRenderer *py3dModelRenderer = (struct Py3dModelRenderer *) PyObject_CallNoArgs(Py3dModelRenderer_Ctor);
    if (py3dModelRenderer == NULL) {
        critical_log("%s", "[Python]: Failed to allocate ModelRendererComponent in python interpreter");
        handleException();

        return NULL;
    }

    return py3dModelRenderer;
}