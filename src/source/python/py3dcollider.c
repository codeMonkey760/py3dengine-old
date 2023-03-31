#include "python/py3dcollider.h"
#include "python/python_util.h"
#include "logger.h"

static PyObject *Py3dCollider_Ctor = NULL;

static PyMethodDef Py3dCollider_Methods[] = {
    {NULL}
};

static int Py3dCollider_Init(struct Py3dCollider *self, PyObject *args, PyObject *kwds) {
    if (Py3dComponent_Type.tp_init((PyObject *) self, args, kwds) == -1) return -1;

    self->geomId = 0;
    self->isTrigger = 0;
}

static void Py3dCollider_Dealloc(struct Py3dCollider *self) {
    Py3dComponent_Dealloc((struct Py3dComponent *) self);
}

static PyTypeObject Py3dCollider_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py3dengine.ColliderComponent",
    .tp_doc = "Detects and responds to collisions with other objects",
    .tp_basicsize = sizeof(struct Py3dCollider),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) Py3dCollider_Init,
    .tp_methods = Py3dCollider_Methods,
    .tp_dealloc = (destructor) Py3dCollider_Dealloc
};

int PyInit_Py3dCollider(PyObject *module) {
    Py3dCollider_Type.tp_base = &Py3dComponent_Type;
    if (PyType_Ready(&Py3dCollider_Type) < 0) return 0;

    if (PyModule_AddObject(module, "ColliderComponent", (PyObject *) &Py3dCollider_Type) < 0) return 0;

    Py_INCREF(&Py3dCollider_Type);

    return 1;
}

int Py3dCollider_FindCtor(PyObject *module) {
    Py3dCollider_Ctor = PyObject_GetAttrString(module, "ColliderComponent");
    if (Py3dCollider_Ctor == NULL) {
        critical_log("%s", "[Python]: Py3dCollider has not been initialized properly");
        handleException();

        return 0;
    }

    return 1;
}

void Py3dCollider_FinalizeCtor() {
    Py_CLEAR(Py3dCollider_Ctor);
}

struct Py3dCollider *Py3dCollider_New() {
    if (Py3dCollider_Ctor == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Py3dCollider not initialized");
        return NULL;
    }

    struct Py3dCollider *py3dCollider = (struct Py3dCollider *) PyObject_CallNoArgs(Py3dCollider_Ctor);
    if (py3dCollider == NULL) {
        critical_log("%s", "[Python]: Failed to allocate Py3dCollider");
        return NULL;
    }

    if (!Py3dCollider_Check((PyObject *) py3dCollider)) {
        PyErr_SetString(PyExc_AssertionError, "Py3dCollider ctor did not return a ColliderComponent");
        Py_CLEAR(py3dCollider);
        return NULL;
    }

    return py3dCollider;
}

int Py3dCollider_Check(PyObject *obj) {
    int ret = PyObject_IsInstance(obj, (PyObject *) &Py3dCollider_Type);
    if (ret == -1) {
        handleException();
        ret = 0;
    }

    return ret;
}