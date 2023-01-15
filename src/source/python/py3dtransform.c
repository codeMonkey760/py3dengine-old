#include "python/py3dtransform.h"
#include "python/python_util.h"
#include "util.h"
#include "logger.h"

static PyObject *py3dTransformCtor = NULL;

static PyMethodDef Py3dTransform_Methods[] = {
    {NULL},
};

static int Py3dTransform_Init(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    if (Py3dComponent_Type.tp_init((PyObject *) self, args, kwds) == -1) return -1;

    Vec3Fill(self->__position, 0.0f);
    QuaternionIdentity(self->__orientation);
    Vec3Fill(self->__scale, 1.0f);
    self->__matrixCacheDirty = true;
    Mat4Identity(self->__wMatrixCache);
    Mat4Identity(self->__witMatrixCache);
    Mat4Identity(self->__viewMatrixCache);
}

static PyTypeObject Py3dTransform_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py3dengine.Transform",
    .tp_doc = "Stores a Games Object's spatial information",
    .tp_basicsize = sizeof(struct Py3dTransform),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) Py3dTransform_Init,
    .tp_methods = Py3dTransform_Methods
};

bool PyInit_Py3dTransform(PyObject *module) {
    Py3dTransform_Type.tp_base = &Py3dComponent_Type;
    if (PyType_Ready(&Py3dTransform_Type) < 0) return false;

    if (PyModule_AddObject(module, "Transform", (PyObject *) &Py3dTransform_Type) < 0) return false;

    Py_INCREF(&Py3dTransform_Type);

    return true;
}

bool Py3dTransform_FindCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "Transform") == 0) {
        critical_log("%s", "[Python]: Py3dTransform has not been initialized properly");

        return false;
    }

    py3dTransformCtor = PyObject_GetAttrString(module, "GameObject");
    Py_INCREF(py3dTransformCtor);

    return true;
}

void Py3dTransform_FinalizeCtor() {
    Py_CLEAR(py3dTransformCtor);
}

PyObject *Py3dTransform_New() {
    if (py3dTransformCtor == NULL) {
        critical_log("%s", "[Python]: Py3dTransform has not been initialized properly");

        return NULL;
    }

    PyObject *py3dtransform = PyObject_CallNoArgs(py3dTransformCtor);
    if (py3dtransform == NULL) {
        critical_log("%s", "[Python]: Failed to allocate Transform in python interpreter");
        handleException();

        return NULL;
    }

    return py3dtransform;
}
