#include "python/py3dcollider.h"
#include "python/python_util.h"
#include "logger.h"

static PyObject *Py3dCollider_Ctor = NULL;

// TODO: finish these
static void deleteGeom() {

}

static void setGeom(dGeomID newGeom) {

}

static const char *getStringFromPyTuple(PyObject *tuple, Py_ssize_t index) {
    PyObject *obj = PyTuple_GetItem(tuple, index);
    if (obj == NULL) {
        PyErr_Clear();
        return NULL;
    }

    if (!PyUnicode_Check(obj)) return NULL;

    return PyUnicode_AsUTF8(obj);
}

static int getRealFromPyTuple(PyObject *tuple, Py_ssize_t index, dReal *result) {
    PyObject *obj = PyTuple_GetItem(tuple, index);
    if (obj == NULL) {
        PyErr_Clear();
        return 0;
    }

    obj = PyNumber_Float(obj);
    if (obj == NULL) {
        PyErr_Clear();
        return 0;
    }

    (*result) = (dReal) PyFloat_AsDouble(obj);
    Py_CLEAR(obj);

    return 1;
}

static PyObject *Py3dCollider_SetShape(struct Py3dCollider *self, PyObject *args, PyObject *kwds) {
    const char *shapeName = getStringFromPyTuple(args, 0);
    if (shapeName == NULL) {
        PyErr_SetString(PyExc_ValueError, "Argument 0 must be a string with a valid collision shape name");
        return NULL;
    }

    dGeomID newGeom = NULL;
    dReal arg1 = 0.0f, arg2 = 0.0f, arg3 = 0.0f;
    if (!getRealFromPyTuple(args, 1, &arg1)) {
        PyErr_SetString(PyExc_ValueError, "Argument 1 must be a number");
        return NULL;
    }

    if (strcmp(shapeName, "SPHERE") == 0) {
        newGeom = dCreateSphere(NULL, arg1);
    } else if (strcmp(shapeName, "CYLINDER") == 0) {
        if (!getRealFromPyTuple(args, 2, &arg2)) {
            PyErr_SetString(PyExc_ValueError, "Argument 2 must be a number");
            return NULL;
        }

        newGeom = dCreateCylinder(NULL, arg1, arg2);
    } else if (strcmp(shapeName, "CAPSULE") == 0) {
        if (!getRealFromPyTuple(args, 2, &arg2)) {
            PyErr_SetString(PyExc_ValueError, "Argument 2 must be a number");
            return NULL;
        }

        newGeom = dCreateCapsule(NULL, arg1, arg2);
    } else if (strcmp(shapeName, "BOX") == 0) {
        if (!getRealFromPyTuple(args, 2, &arg2)) {
            PyErr_SetString(PyExc_ValueError, "Argument 2 must be a number");
            return NULL;
        }
        if (!getRealFromPyTuple(args, 3, &arg3)) {
            PyErr_SetString(PyExc_ValueError, "Argument 3 must be a number");
            return NULL;
        }

        newGeom = dCreateBox(NULL, arg1, arg2, arg3);
    } else {
        PyErr_SetString(PyExc_ValueError, "Argument 0 must be a string with a valid collision shape name");
        return NULL;
    }

    deleteGeom();
    setGeom(newGeom);
    Py_RETURN_NONE;
}

static PyMethodDef Py3dCollider_Methods[] = {
    {"set_shape", (PyCFunction) Py3dCollider_SetShape, METH_VARARGS, "Set collision shape"},
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