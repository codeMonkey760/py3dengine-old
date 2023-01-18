#include "math/vector3.h"
#include "logger.h"
#include "python/python_util.h"

struct Py3dVector3 {
    PyObject_HEAD
    float elements[3];
};

static PyObject *py3dVector3Ctor = NULL;

static int setComponent(struct Py3dVector3 *self, PyObject *value, int index) {
    char names[6] = {'x',0,'y',0,'z',0};

    if (value == NULL) {
        PyErr_Format(PyExc_TypeError, "Cannot delete the \"%s\" attribute", &names[index * 2]);
        return -1;
    }

    Py_INCREF(value);
    PyObject *flt = PyNumber_Float(value);
    if (flt == NULL) {
        PyErr_Format(PyExc_TypeError, "The \"%s\" attribute must be convertible to float", &names[index * 2]);
        Py_DECREF(value);
        return -1;
    }

    self->elements[index] = (float) PyFloat_AsDouble(flt);
    Py_DECREF(flt);
    Py_DECREF(value);

    return 0;
}

static void Py3dVector3_Dealloc(struct Py3dVector3 *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dVector3_Init(struct Py3dVector3 *self, PyObject *args, PyObject *kwds) {
    self->elements[0] = 0.0f;
    self->elements[1] = 0.0f;
    self->elements[2] = 0.0f;

    return 0;
}

static PyObject *Py3dVector3_GetX(struct Py3dVector3 *self, void *closure) {
    return PyFloat_FromDouble(self->elements[0]);
}

static int Py3dVector3_SetX(struct Py3dVector3 *self, PyObject *value, void *closure) {
    return setComponent(self, value, 0);
}

static PyObject *Py3dVector3_GetY(struct Py3dVector3 *self, void *closure) {
    return PyFloat_FromDouble(self->elements[1]);
}

static int Py3dVector3_SetY(struct Py3dVector3 *self, PyObject *value, void *closure) {
    return setComponent(self, value, 1);
}

static PyObject *Py3dVector3_GetZ(struct Py3dVector3 *self, void *closure) {
    return PyFloat_FromDouble(self->elements[2]);
}

static int Py3dVector3_SetZ(struct Py3dVector3 *self, PyObject *value, void *closure) {
    return setComponent(self, value, 2);
}

static PyObject *Py3dVector3_Repr(struct Py3dVector3 *self) {
    // TODO: this is hacky, but PyUnicode_FromFormat won't print floats so I blame python 3.10
    char buffer[65];
    memset(buffer, 0, 65 * sizeof(char));
    snprintf(buffer, 64, "Vector3(%.2f, %.2f, %.2f)", self->elements[0], self->elements[1], self->elements[2]);

    PyUnicode_FromString(buffer);
}

PyGetSetDef Py3dVector3_GettersSetters[] = {
    {"x", (getter) Py3dVector3_GetX, (setter) Py3dVector3_SetX, "X Component of Vector3", NULL},
    {"y", (getter) Py3dVector3_GetY, (setter) Py3dVector3_SetY, "Y Component of Vector3", NULL},
    {"z", (getter) Py3dVector3_GetZ, (setter) Py3dVector3_SetZ, "Z Component of Vector3", NULL},
    {NULL}
};

PyMethodDef Py3dVector3_Methods[] = {
    {NULL}
};

PyTypeObject Py3dVector3_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py3dengine.math.Vector3",
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