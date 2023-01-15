#include "python/py3dtransform.h"
#include "python/python_util.h"
#include "util.h"
#include "logger.h"

static PyObject *py3dTransformCtor = NULL;

static PyObject *createVecDict(float x, float y, float z, float w, int length) {
    PyObject *ret = PyDict_New();
    if (ret == NULL) return NULL;

    char elementNames[8] = {'x', 0, 'y', 0, 'z', 0, 'w', 0};
    float elementValues[4] = {x, y, z, w};

    for (int i = 0; i < length; ++i) {
        PyObject *curFloat = PyFloat_FromDouble(elementValues[i]);
        if (curFloat == NULL) {
            Py_CLEAR(ret);
            return NULL;
        }

        if (PyDict_SetItemString(ret, &(elementNames[i*2]), curFloat) == -1) {
            Py_CLEAR(ret);
            Py_CLEAR(curFloat);
            return NULL;
        }

        // PyDict_SetItemX is weird ... it doesn't steal references from caller, so we have to clean up
        Py_CLEAR(curFloat);
        curFloat = NULL;
    }

    return ret;
}

static PyMethodDef Py3dTransform_Methods[] = {
    {NULL},
};

static int Py3dTransform_Init(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    if (Py3dComponent_Type.tp_init((PyObject *) self, args, kwds) == -1) return -1;

    self->__position = createVecDict(0.0f, 0.0f, 0.0f, 0.0f, 3);
    self->__orientation = createVecDict(0.0f, 0.0f, 0.0f, 1.0f, 4);
    self->__scale = createVecDict(1.0f, 1.0f, 1.0f, 0.0f, 3);
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
