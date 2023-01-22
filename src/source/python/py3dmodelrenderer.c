#include "python/py3dmodelrenderer.h"
#include "logger.h"
#include "python/python_util.h"
#include "python/py3dtransform.h"
#include "game_object.h"

static PyObject *Py3dModelRenderer_Ctor = NULL;

static struct Py3dTransform *getTransform(struct Py3dModelRenderer *self) {
    PyObject *get_owner = PyObject_GetAttrString((PyObject *) self, "get_owner");
    if (get_owner == NULL) {
        PyErr_SetString(PyExc_ValueError, "ModelRenderComponent does not have a \"get_owner\" attribute");
        return NULL;
    } else if (PyCallable_Check(get_owner) != 1) {
        PyErr_SetString(PyExc_ValueError, "ModelRenderComponent's \"get_owner\" attribute is not callable");
        Py_CLEAR(get_owner);
        return NULL;
    }

    PyObject *ownerObj = PyObject_CallNoArgs(get_owner);
    Py_CLEAR(get_owner);
    if (ownerObj == NULL) {
        return NULL;
    } else if (PyObject_IsInstance(ownerObj, &Py3dGameObject_Type) != 1) {
        PyErr_SetString(PyExc_ValueError, "ModelRenderComponent's owner is improperly set");
        return NULL;
    }
    struct Py3dGameObject *pyOwner = (struct Py3dGameObject *) ownerObj;
    if (pyOwner->gameObject == NULL) {
        PyErr_Set()
    }
}

static PyObject *Py3dModelRenderer_Render(struct Py3dModelRenderer *self, PyObject *args, PyObject *kwds) {
    if (self->shader == NULL || self->model == NULL || self->material == NULL) {
        PyErr_SetString(PyExc_ValueError, "ModelRendererComponent is not correctly configured");
        return NULL;
    }

    // TODO: This is an atrocity ... I should be using the python functions get_owner and get_transform to do this
    if (self->base.owner == NULL || self->base.owner->transform) {
        PyErr_SetString(PyExc_ValueError, "ModelRendererComponent cannot retrieve owner's transform");
    }
    struct Py3dTransform *transform = self->base.owner->transform;



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