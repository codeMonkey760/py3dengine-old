#include "math/vector3.h"
#include "logger.h"
#include "python/python_util.h"

struct Py3dVector3 {
    PyObject_HEAD
    float elements[3];
};

static PyObject *py3dVector3Ctor = NULL;

static void Py3dVector3_Dealloc(struct Py3dVector3 *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dVector3_Init(struct Py3dVector3 *self, PyObject *args, PyObject *kwds) {
    self->elements[0] = 0.0f;
    self->elements[1] = 0.0f;
    self->elements[2] = 0.0f;

    return 0;
}

PyMethodDef Py3dVector3_Methods[] = {
    {NULL}
};

PyTypeObject Py3dVector3_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py3dengine.math.Vector3",
    .tp_doc = "A 3 dimensional vector",
    .tp_basicsize = sizeof(struct Py3dVector3),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) Py3dVector3_Init,
    .tp_methods = Py3dVector3_Methods,
    .tp_dealloc = (destructor) Py3dVector3_Dealloc,
    .tp_new = PyType_GenericNew
};

bool PyInit_Py3dVector3(PyObject *module) {
    if (PyType_Ready(&Py3dVector3_Type) < 0) return false;

    if (PyModule_AddObject(module, "Vector3", (PyObject *) &Py3dVector3_Type) < 0) return false;
    Py_INCREF(&Py3dVector3_Type);

    return true;
}

bool Py3dPy3dVector3_FindCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "Vector3") == 0) {
        critical_log("%s", "[Python]: Py3dVector3 has not been initialized properly");

        return false;
    }

    py3dVector3Ctor = PyObject_GetAttrString(module, "Vector3");

    return true;
}

void Py3dVector3_FinalizeCtor() {
    Py_CLEAR(py3dVector3Ctor);
}

struct Py3dVector3 *Py3dVector3_New() {
    if (py3dVector3Ctor == NULL) {
        critical_log("%s", "[Python]: Py3dVector3 has not been initialized properly");

        return NULL;
    }

    PyObject *py3dVector3 = PyObject_CallNoArgs(py3dVector3Ctor);
    if (py3dVector3 == NULL) {
        critical_log("%s", "[Python]: Failed to allocate Vector3 in python interpreter");
        handleException();

        return NULL;
    }

    return (struct Py3dVector3 *) py3dVector3;
}