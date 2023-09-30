#include "math/quaternion.h"
#include "math/vector3.h"
#include "logger.h"

static PyObject *py3dQuaternionCtor = NULL;

static void Py3dQuaternion_Dealloc(struct Py3dQuaternion *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dQuaternion_Init(struct Py3dQuaternion *self, PyObject *args, PyObject *kwds) {
    if (PyTuple_Size(args) == 0) {
        self->elements[0] = 0.0f;
        self->elements[1] = 0.0f;
        self->elements[2] = 0.0f;
        self->elements[3] = 1.0f;

        return 0;
    }

    float t1 = 0.0f;
    float t2 = 0.0f;
    float t3 = 0.0f;
    float t4 = 1.0f;

    if (PyArg_ParseTuple(args, "ffff", &t1, &t2, &t3, &t4) != 1) return -1;

    self->elements[0] = t1;
    self->elements[1] = t2;
    self->elements[2] = t3;
    self->elements[3] = t4;

    return 0;
}

static PyObject *Py3dQuaternion_GetX(struct Py3dQuaternion *self, void *closure) {
    return PyFloat_FromDouble(self->elements[0]);
}

static PyObject *Py3dQuaternion_GetY(struct Py3dQuaternion *self, void *closure) {
    return PyFloat_FromDouble(self->elements[1]);
}

static PyObject *Py3dQuaternion_GetZ(struct Py3dQuaternion *self, void *closure) {
    return PyFloat_FromDouble(self->elements[2]);
}

static PyObject *Py3dQuaternion_GetW(struct Py3dQuaternion *self, void *closure) {
    return PyFloat_FromDouble(self->elements[3]);
}

static PyObject *Py3dQuaternion_Repr(struct Py3dQuaternion *self) {
    char buffer[65];
    memset(buffer, 0, 65 * sizeof(char));
    snprintf(
        buffer, 64, "Quaternion(%.2f, %.2f, %.2f, %.2f)",
        self->elements[0],
        self->elements[1],
        self->elements[2],
        self->elements[3]
    );

    return PyUnicode_FromString(buffer);
}

static PyObject *Py3dQuaternion_Mult(struct Py3dQuaternion *self, PyObject *other) {
    int isQuaternion = PyObject_IsInstance(other, (PyObject *) &Py3dQuaternion_Type);
    if (isQuaternion == -1) return NULL;

    if (isQuaternion == 1) {
        struct Py3dQuaternion *otherQuaternion = (struct Py3dQuaternion *) other;

        float x =
            (self->elements[3] * otherQuaternion->elements[0]) +
            (self->elements[0] * otherQuaternion->elements[3]) +
            (self->elements[1] * otherQuaternion->elements[2]) -
            (self->elements[2] * otherQuaternion->elements[1]);

        float y =
            (self->elements[3] * otherQuaternion->elements[1]) -
            (self->elements[0] * otherQuaternion->elements[2]) +
            (self->elements[1] * otherQuaternion->elements[3]) +
            (self->elements[2] * otherQuaternion->elements[0]);

        float z =
            (self->elements[3] * otherQuaternion->elements[2]) +
            (self->elements[0] * otherQuaternion->elements[1]) -
            (self->elements[1] * otherQuaternion->elements[0]) +
            (self->elements[2] * otherQuaternion->elements[3]);

        float w =
            (self->elements[3] * otherQuaternion->elements[3]) -
            (self->elements[0] * otherQuaternion->elements[0]) -
            (self->elements[1] * otherQuaternion->elements[1]) -
            (self->elements[2] * otherQuaternion->elements[2]);

        return (PyObject *) Py3dQuaternion_New(x, y, z, w);
    }

    PyErr_SetString(PyExc_TypeError, "Second operand must be of type Vector3 or Quaternion");
    return NULL;
}

static PyObject *Py3dQuaternion_Normalize(struct Py3dQuaternion *self, PyObject *args, PyObject *kwds) {
    float length = sqrtf(
        (self->elements[0] * self->elements[0]) +
        (self->elements[1] * self->elements[1]) +
        (self->elements[2] * self->elements[2]) +
        (self->elements[3] * self->elements[3])
    );

    if (length == 0.0f) {
        PyErr_SetString(PyExc_ZeroDivisionError, "Cannot normalize a quaternion with a length of zero");
        return NULL;
    }

    float x = self->elements[0] / length;
    float y = self->elements[1] / length;
    float z = self->elements[2] / length;
    float w = self->elements[3] / length;

    return (PyObject *) Py3dQuaternion_New(x, y, z, w);
}

static PyObject *Py3dQuaternion_FromAxisAndDegrees(struct Py3dQuaternion *self, PyObject *args, PyObject *kwds) {
    struct Py3dVector3 *axis = NULL;
    float angle = 0.0f;
    if (PyArg_ParseTuple(args, "O!f", &Py3dVector3_Type, &axis, &angle) != 1) return NULL;

    angle *= (float) 0.017453293; //convert to radians
    float fac = sinf(angle / 2.0f);

    float x = axis->elements[0] * fac;
    float y = axis->elements[1] * fac;
    float z = axis->elements[2] * fac;
    float w = cosf(angle / 2.0f);

    float length = sqrtf((x * x) + (y * y) + (z * z) + (w * w));

    x /= length;
    y /= length;
    z /= length;
    w /= length;

    return (PyObject *) Py3dQuaternion_New(x, y, z, w);
}

PyGetSetDef Py3dQuaternion_GettersSetters[] = {
    {"x", (getter) Py3dQuaternion_GetX, (setter) NULL, "X Component of Quaternion", NULL},
    {"y", (getter) Py3dQuaternion_GetY, (setter) NULL, "Y Component of Quaternion", NULL},
    {"z", (getter) Py3dQuaternion_GetZ, (setter) NULL, "Z Component of Quaternion", NULL},
    {"w", (getter) Py3dQuaternion_GetW, (setter) NULL, "W Component of Quaternion", NULL},
    {NULL}
};

PyMethodDef Py3dQuaternion_Methods[] = {
    {"FromAxisAndDegrees", (PyCFunction) Py3dQuaternion_FromAxisAndDegrees, METH_VARARGS| METH_STATIC, "Create a Quaternion from an axis and a angle in degrees"},
    {"normalize", (PyCFunction) Py3dQuaternion_Normalize, METH_NOARGS, "Return a normalized version of the quaterion"},
    {NULL}
};

PyNumberMethods Py3dQuaternion_NumberMethods = {
    .nb_multiply = (binaryfunc) Py3dQuaternion_Mult,
};

PyTypeObject Py3dQuaternion_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "py3dmath.Quaternion",
        .tp_doc = "A quaternion",
        .tp_basicsize = sizeof(struct Py3dQuaternion),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_init = (initproc) Py3dQuaternion_Init,
        .tp_methods = Py3dQuaternion_Methods,
        .tp_dealloc = (destructor) Py3dQuaternion_Dealloc,
        .tp_new = PyType_GenericNew,
        .tp_repr = (reprfunc) Py3dQuaternion_Repr,
        .tp_str = (reprfunc) Py3dQuaternion_Repr,
        .tp_getset = Py3dQuaternion_GettersSetters
};

bool PyInit_Py3dQuaternion(PyObject *module) {
    Py3dQuaternion_Type.tp_as_number = &Py3dQuaternion_NumberMethods;
    if (PyType_Ready(&Py3dQuaternion_Type) < 0) return false;

    if (PyModule_AddObject(module, "Quaternion", (PyObject *) &Py3dQuaternion_Type) < 0) return false;
    Py_INCREF(&Py3dQuaternion_Type);

    return true;
}

bool Py3dQuaternion_FindCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "Quaternion") == 0) {
        critical_log("%s", "[Python]: Py3dQuaternion has not been initialized properly");

        return false;
    }

    py3dQuaternionCtor = PyObject_GetAttrString(module, "Quaternion");

    return true;
}

void Py3dQuaternion_FinalizeCtor() {
    Py_CLEAR(py3dQuaternionCtor);
}

struct Py3dQuaternion *Py3dQuaternion_New(float x, float y, float z, float w) {
    if (py3dQuaternionCtor == NULL) {
        critical_log("%s", "[Python]: Py3dQuaternion has not been initialized properly");

        return NULL;
    }

    PyObject *args = Py_BuildValue("(ffff)", x, y, z, w);
    struct Py3dQuaternion *ret = (struct Py3dQuaternion *) PyObject_Call(py3dQuaternionCtor, args, NULL);
    Py_CLEAR(args);

    return ret;
}
