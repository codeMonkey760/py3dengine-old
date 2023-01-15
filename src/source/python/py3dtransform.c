#include "python/py3dtransform.h"
#include "python/python_util.h"
#include "util.h"
#include "logger.h"

static PyObject *py3dTransformCtor = NULL;

static PyObject *makeTupleFromFloatArray(const float *src, int length) {
    PyObject *ret = PyTuple_New(length);
    if (ret == NULL) return NULL;

    for (int i = 0; i < length; ++i) {
        if (PyTuple_SetItem(ret, i, PyFloat_FromDouble(src[i])) == -1) {
            Py_CLEAR(ret);
            return NULL;
        }
    }

    return ret;
}

static bool unpackNumberTupleIntoFloatArray(PyObject *tuple, unsigned int required_elements, float *dst) {
    if (tuple == NULL || required_elements == 0 || dst == NULL) return false;

    if (PyTuple_Check(tuple) != 1) return false;

    Py_ssize_t tuple_len = PyTuple_Size(tuple);
    if (tuple_len < required_elements) return false;

    for (unsigned int i = 0; i < required_elements; ++i) {
        PyObject *curElement = PyTuple_GetItem(tuple, i);
        curElement = PyNumber_Float(curElement);
        if (curElement == NULL) return false;

        dst[i] = (float) PyFloat_AsDouble(curElement);
        Py_CLEAR(curElement);
    }

    return true;
}

static PyObject *Py3dTransform_GetPosition(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    PyObject *ret = makeTupleFromFloatArray(self->__position, 3);
    if (ret == NULL) {
        ret = Py_None;
    }

    Py_INCREF(ret);
    return ret;
}

static PyObject *Py3dTransform_Move(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    float temp[3] = {0.0f};
    if (!unpackNumberTupleIntoFloatArray(args, 3, temp)) {
        PyErr_SetString(PyExc_ValueError, "Transform.move requires 3 floats as arguments");
        return NULL;
    }

    Vec3Add(self->__position, self->__position, temp);
    self->__matrixCacheDirty = true;

    Py_RETURN_NONE;
}

static PyObject *Py3dTransform_SetPosition(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    float temp[3] = {0.0f};
    if (!unpackNumberTupleIntoFloatArray(args, 3, temp)) {
        PyErr_SetString(PyExc_ValueError, "Transform.set_position requires 3 floats as arguments");
        return NULL;
    }

    Vec3Copy(self->__position, temp);
    self->__matrixCacheDirty = true;

    Py_RETURN_NONE;
}

static PyObject *Py3dTransform_GetOrientation(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    PyObject *ret = makeTupleFromFloatArray(self->__orientation, 4);
    if (ret == NULL) {
        ret = Py_None;
    }

    Py_INCREF(ret);
    return ret;
}

static PyObject *Py3dTransform_Rotate(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    float temp[4] = {0.0f};
    if (!unpackNumberTupleIntoFloatArray(args, 4, temp)) {
        PyErr_SetString(PyExc_ValueError, "Transform.rotate requires 4 floats as arguments");
        return NULL;
    }

    QuaternionMult(self->__orientation, temp, self->__orientation);
    self->__matrixCacheDirty = true;

    Py_RETURN_NONE;
}

static PyObject *Py3dTransform_SetOrientation(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    float temp[4] = {0.0f};
    if (!unpackNumberTupleIntoFloatArray(args, 4, temp)) {
        PyErr_SetString(PyExc_ValueError, "Transform.set_orientation requires 4 floats as arguments");
        return NULL;
    }

    QuaternionCopy(self->__orientation, temp);
    self->__matrixCacheDirty = true;

    Py_RETURN_NONE;
}

static PyObject *Py3dTransform_GetScale(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    PyObject *ret = makeTupleFromFloatArray(self->__scale, 3);
    if (ret == NULL) {
        ret = Py_None;
    }

    Py_INCREF(ret);
    return ret;
}

static PyObject *Py3dTransform_Stretch(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    float temp[3] = {0.0f};
    if (!unpackNumberTupleIntoFloatArray(args, 3, temp)) {
        PyErr_SetString(PyExc_ValueError, "Transform.stretch requires 3 floats as arguments");
        return NULL;
    }

    self->__scale[0] *= temp[0];
    self->__scale[1] *= temp[1];
    self->__scale[2] *= temp[2];
    self->__matrixCacheDirty = true;

    Py_RETURN_NONE;
}

static PyObject *Py3dTransform_SetScale(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    float temp[3] = {0.0f};
    if (!unpackNumberTupleIntoFloatArray(args, 3, temp)) {
        PyErr_SetString(PyExc_ValueError, "Transform.set_scale requires 3 floats as arguments");
        return NULL;
    }

    Vec3Copy(self->__scale, temp);
    self->__matrixCacheDirty = true;

    Py_RETURN_NONE;
}

static PyMethodDef Py3dTransform_Methods[] = {
    {"get_position", (PyCFunction) Py3dTransform_GetPosition, METH_NOARGS, "Get position as tuple"},
    {"move", (PyCFunction) Py3dTransform_Move, METH_VARARGS, "Move the transform by displacement value"},
    {"set_position", (PyCFunction) Py3dTransform_SetPosition, METH_VARARGS, "Set the position by absolute value"},
    {"get_orientation", (PyCFunction) Py3dTransform_GetOrientation, METH_NOARGS, "Get orientation as tuple"},
    {"rotate", (PyCFunction) Py3dTransform_Rotate, METH_VARARGS, "Rotate the transform by displacement value"},
    {"set_orientation", (PyCFunction) Py3dTransform_SetOrientation, METH_VARARGS, "Set the orientation by absolute value"},
    {"get_scale", (PyCFunction) Py3dTransform_GetScale, METH_NOARGS, "Get scale as tuple"},
    {"stretch", (PyCFunction) Py3dTransform_Stretch, METH_VARARGS, "Stretch the transform by factor value"},
    {"set_scale", (PyCFunction) Py3dTransform_SetScale, METH_VARARGS, "Set the scale by absolute value"},
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

    return 0;
}

static PyTypeObject Py3dTransform_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py3dengine.Transform",
    .tp_doc = "Stores a Games Object's spatial information",
    .tp_basicsize = sizeof(struct Py3dTransform),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) Py3dTransform_Init,
    .tp_methods = Py3dTransform_Methods,
    .tp_new = PyType_GenericNew
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
