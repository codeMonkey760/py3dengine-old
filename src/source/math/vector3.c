#include "math/vector3.h"
#include "logger.h"
#include "python/python_util.h"

struct Py3dVector3 {
    PyObject_HEAD
    float elements[3];
};

static PyObject *py3dVector3Ctor = NULL;
PyTypeObject Py3dVector3_Type;

static void Py3dVector3_Dealloc(struct Py3dVector3 *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dVector3_Init(struct Py3dVector3 *self, PyObject *args, PyObject *kwds) {
    if (PyTuple_Size(args) == 0) {
        self->elements[0] = 0.0f;
        self->elements[1] = 0.0f;
        self->elements[2] = 0.0f;

        return 0;
    }

    float t1 = 0.0f;
    float t2 = 0.0f;
    float t3 = 0.0f;

    if (PyArg_ParseTuple(args, "fff", &t1, &t2, &t3) == 1) {
        self->elements[0] = t1;
        self->elements[1] = t2;
        self->elements[2] = t3;

        return 0;
    }

    PyErr_Clear();
    if (PyArg_ParseTuple(args, "f", &t1) == 1) {
        self->elements[0] = t1;
        self->elements[1] = t1;
        self->elements[2] = t1;

        return 0;
    }

    PyErr_Clear();
    PyErr_SetString(PyExc_ValueError, "Vector3 constructor expected zero floats, one float, or three floats");

    return -1;
}

static PyObject *Py3dVector3_GetX(struct Py3dVector3 *self, void *closure) {
    return PyFloat_FromDouble(self->elements[0]);
}

static PyObject *Py3dVector3_GetY(struct Py3dVector3 *self, void *closure) {
    return PyFloat_FromDouble(self->elements[1]);
}

static PyObject *Py3dVector3_GetZ(struct Py3dVector3 *self, void *closure) {
    return PyFloat_FromDouble(self->elements[2]);
}

static PyObject *Py3dVector3_Repr(struct Py3dVector3 *self) {
    // TODO: this is hacky, but PyUnicode_FromFormat won't print floats so I blame python 3.10
    char buffer[65];
    memset(buffer, 0, 65 * sizeof(char));
    snprintf(buffer, 64, "Vector3(%.2f, %.2f, %.2f)", self->elements[0], self->elements[1], self->elements[2]);

    PyUnicode_FromString(buffer);
}

static struct Py3dVector3 *checkTypeOfOther(PyObject *other) {
    int isInstanceCheck = PyObject_IsInstance(other, (PyObject *) &Py3dVector3_Type);
    if (isInstanceCheck == -1) {
        return NULL;
    } else if (isInstanceCheck == 0) {
        PyErr_SetString(PyExc_TypeError, "Vector3 can only be added to other Vector3 instances");
        return NULL;
    }

    return (struct Py3dVector3 *) other;
}

static PyObject *Py3dVector3_Add(struct Py3dVector3 *self, PyObject *other) {
    struct Py3dVector3 *otherAsVec3 = checkTypeOfOther(other);
    if (otherAsVec3 == NULL) return NULL;

    struct Py3dVector3 *result = Py3dVector3_New();
    result->elements[0] = self->elements[0] + otherAsVec3->elements[0];
    result->elements[1] = self->elements[1] + otherAsVec3->elements[1];
    result->elements[2] = self->elements[2] + otherAsVec3->elements[2];

    return (PyObject *) result;
}

static PyObject *Py3dVector3_Sub(struct Py3dVector3 *self, PyObject *other) {
    struct Py3dVector3 *otherAsVec3 = checkTypeOfOther(other);
    if (otherAsVec3 == NULL) return NULL;

    struct Py3dVector3 *result = Py3dVector3_New();
    result->elements[0] = self->elements[0] - otherAsVec3->elements[0];
    result->elements[1] = self->elements[1] - otherAsVec3->elements[1];
    result->elements[2] = self->elements[2] - otherAsVec3->elements[2];

    return (PyObject *) result;
}

static struct Py3dVector3 *do_PyVector3_Cross_Mult(struct Py3dVector3 *self, struct Py3dVector3 *other) {
    struct Py3dVector3 *result = Py3dVector3_New();

    result->elements[0] = (self->elements[1] * other->elements[2]) - (self->elements[2] * other->elements[1]);
    result->elements[1] = (self->elements[2] * other->elements[0]) - (self->elements[0] * other->elements[2]);
    result->elements[2] = (self->elements[0] * other->elements[1]) - (self->elements[1] * other->elements[0]);

    return result;
}

static struct Py3dVector3 *do_PyVector3_Scalar_Mult(struct Py3dVector3 *self, float scalar) {
    struct Py3dVector3 *result = Py3dVector3_New();

    result->elements[0] = self->elements[0] * scalar;
    result->elements[1] = self->elements[1] * scalar;
    result->elements[2] = self->elements[2] * scalar;

    return result;
}

static PyObject *Py3dVector3_Mult(struct Py3dVector3 *self, PyObject *other) {
    int isPy3dVector3 = PyObject_IsInstance(other, (PyObject *) &Py3dVector3_Type);
    if (isPy3dVector3 == -1) {
        return NULL;
    } else if (isPy3dVector3 == 1) {
        return  (PyObject *) do_PyVector3_Cross_Mult(self, (struct Py3dVector3 *) other);
    }

    PyObject *otherAsFlt = PyNumber_Float(other);
    if (otherAsFlt == NULL) {
        PyErr_SetString(PyExc_TypeError, "Vector3 can only be cross multiplied by Vector3 instances or scaled by numbers");
        return NULL;
    }

    PyObject *result = (PyObject *) do_PyVector3_Scalar_Mult(self, (float) PyFloat_AsDouble(otherAsFlt));
    Py_CLEAR(otherAsFlt);
    return result;
}

static PyObject *Py3dVector3_Div(struct Py3dVector3 *self, PyObject *other) {
    PyObject *otherAsFlt = PyNumber_Float(other);
    if (otherAsFlt == NULL) {
        PyErr_SetString(PyExc_TypeError, "Vector3 can only be divided by numbers");
        return NULL;
    }

    float scalar = (float) PyFloat_AsDouble(otherAsFlt);
    Py_CLEAR(otherAsFlt);

    if (scalar == 0.0f) {
        PyErr_SetString(PyExc_ZeroDivisionError, "Vector3 cannot be divided by zero");
        return NULL;
    }

    struct Py3dVector3 *result = Py3dVector3_New();
    result->elements[0] = self->elements[0] / scalar;
    result->elements[1] = self->elements[1] / scalar;
    result->elements[2] = self->elements[2] / scalar;

    return (PyObject *) result;
}

static PyObject *Py3dVector3_Dot(struct Py3dVector3 *self, PyObject *args, PyObject *kwds) {
    struct Py3dVector3 *other = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dVector3_Type, &other) == 0) return NULL;

    return PyFloat_FromDouble(
        (self->elements[0] * other->elements[0]) +
        (self->elements[1] * other->elements[1]) +
        (self->elements[2] * other->elements[2])
    );
}

static float do_length(struct Py3dVector3 *self) {
    return sqrtf(
            (self->elements[0] * self->elements[0]) +
            (self->elements[1] * self->elements[1]) +
            (self->elements[2] * self->elements[2])
    );
}

static PyObject *Py3dVector3_Length(struct Py3dVector3 *self, PyObject *args, PyObject *kwds) {
    return PyFloat_FromDouble(do_length(self));
}

static PyObject *Py3dVector3_Normalize(struct Py3dVector3 *self, PyObject *args, PyObject *kwds) {
    float length = do_length(self);
    if (length == 0.0f) {
        PyErr_SetString(PyExc_ZeroDivisionError, "Cannot normalize Vector3 of length zero.");
        return NULL;
    }

    struct Py3dVector3 *result = Py3dVector3_New();
    result->elements[0] = self->elements[0] / length;
    result->elements[1] = self->elements[1] / length;
    result->elements[2] = self->elements[2] / length;

    return (PyObject *) result;
}

PyGetSetDef Py3dVector3_GettersSetters[] = {
    {"x", (getter) Py3dVector3_GetX, (setter) NULL, "X Component of Vector3", NULL},
    {"y", (getter) Py3dVector3_GetY, (setter) NULL, "Y Component of Vector3", NULL},
    {"z", (getter) Py3dVector3_GetZ, (setter) NULL, "Z Component of Vector3", NULL},
    {NULL}
};

PyMethodDef Py3dVector3_Methods[] = {
    {"dot", (PyCFunction) Py3dVector3_Dot, METH_VARARGS, "Return the dot product of two Vector3 instances"},
    {"length", (PyCFunction) Py3dVector3_Length, METH_NOARGS, "Return the length of a Vector3 instance"},
    {"normalize", (PyCFunction) Py3dVector3_Normalize, METH_NOARGS, "Return the normalized version of a Vector3 instance"},
    {NULL}
};

PyNumberMethods Py3dVector3_NumberMethods = {
    .nb_add = (binaryfunc) Py3dVector3_Add,
    .nb_subtract = (binaryfunc) Py3dVector3_Sub,
    .nb_multiply = (binaryfunc) Py3dVector3_Mult,
    .nb_true_divide = (binaryfunc) Py3dVector3_Div
};

PyTypeObject Py3dVector3_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py3dmath.Vector3",
    .tp_doc = "A 3 dimensional vector",
    .tp_basicsize = sizeof(struct Py3dVector3),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_init = (initproc) Py3dVector3_Init,
    .tp_methods = Py3dVector3_Methods,
    .tp_dealloc = (destructor) Py3dVector3_Dealloc,
    .tp_new = PyType_GenericNew,
    .tp_repr = (reprfunc) Py3dVector3_Repr,
    .tp_str = (reprfunc) Py3dVector3_Repr,
    .tp_getset = Py3dVector3_GettersSetters
};

bool PyInit_Py3dVector3(PyObject *module) {
    Py3dVector3_Type.tp_as_number = &Py3dVector3_NumberMethods;
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