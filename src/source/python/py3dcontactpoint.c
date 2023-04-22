#include "python/py3dcontactpoint.h"
#include "math/vector3.h"
#include "python/python_util.h"
#include "logger.h"

static PyObject *Py3dContactPoint_Ctor = NULL;

static int Py3dContactPoint_Init(struct Py3dContactPoint *self, PyObject *args, PyObject *kwds) {
    self->position = Py_NewRef(Py_None);
    self->normal = Py_NewRef(Py_None);
    self->depth = Py_NewRef(Py_None);

    return 1;
}

static void Py3dContactPoint_Dealloc(struct Py3dContactPoint *self) {
    Py_CLEAR(self->position);
    Py_CLEAR(self->normal);
    Py_CLEAR(self->depth);
}

static PyGetSetDef Py3dContactPoint_GetSet[] = {
    {"position", (getter) Py3dContactPoint_GetPosition, NULL, "Retrieve position of contact point as a Vector3", NULL},
    {"normal", (getter) Py3dContactPoint_GetNormal, NULL, "Retrieve direction of contact point as a Vector3", NULL},
    {"depth", (getter) Py3dContactPoint_GetDepth, NULL, "Retrieve depth of contact point as a float", NULL},
    NULL
};

static PyTypeObject Py3dContactPoint_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py3dengine.ContactPoint",
    .tp_doc = "Represents a point of contact during a collision",
    .tp_basicsize = sizeof(struct Py3dContactPoint),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) Py3dContactPoint_Init,
    .tp_dealloc = (destructor) Py3dContactPoint_Dealloc,
    .tp_getset = Py3dContactPoint_GetSet
};

int PyInit_Py3dContactPoint(PyObject *module) {
    if (PyType_Ready(&Py3dContactPoint_Type) < 0) return 0;

    if (PyModule_AddObject(module, "ContactPoint", (PyObject *) &Py3dContactPoint_Type) < 0) return 0;

    Py_INCREF(&Py3dContactPoint_Type);

    return 1;
}

int Py3dContactPoint_FindCtor(PyObject *module) {
    Py3dContactPoint_Ctor = PyObject_GetAttrString(module, "ContactPoint");
    if (Py3dContactPoint_Ctor == NULL) {
        critical_log("%s", "[Python]: Py3dContactPoint has not been initialized properly");
        handleException();

        return 0;
    }

    return 1;
}

void Py3dContactPoint_FinalizeCtor() {
    Py_CLEAR(Py3dContactPoint_Ctor);
}

struct Py3dContactPoint *Py3dContactPoint_New(dContactGeom *odeContactPoint) {
    if (Py3dContactPoint_Ctor == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Py3dContactPoint not initialized");
        return NULL;
    }

    struct Py3dContactPoint *py3dContactPoint = (struct Py3dContactPoint *) PyObject_CallNoArgs(Py3dContactPoint_Ctor);
    if (py3dContactPoint == NULL) {
        critical_log("%s", "[Python]: Failed to allocate Py3dContactPoint");
        return NULL;
    }

    if (!Py3dContactPoint_Check((PyObject *) py3dContactPoint)) {
        PyErr_SetString(PyExc_AssertionError, "Py3dContactPoint ctor did not return a ContactPoint");
        Py_CLEAR(py3dContactPoint);
        return NULL;
    }

    Py_CLEAR(py3dContactPoint->position);
    py3dContactPoint->position = (PyObject *) Py3dVector3_New(
        (float) odeContactPoint->pos[0],
        (float) odeContactPoint->pos[1],
        (float) odeContactPoint->pos[2]
    );

    Py_CLEAR(py3dContactPoint->normal);
    py3dContactPoint->normal = (PyObject *) Py3dVector3_New(
            (float) odeContactPoint->normal[0],
            (float) odeContactPoint->normal[1],
            (float) odeContactPoint->normal[2]
    );

    Py_CLEAR(py3dContactPoint->depth);
    py3dContactPoint->depth = PyFloat_FromDouble(odeContactPoint->depth);

    return py3dContactPoint;
}

int Py3dContactPoint_Check(PyObject *obj) {
    int ret = PyObject_IsInstance(obj, (PyObject *) &Py3dContactPoint_Type);
    if (ret == -1) {
        handleException();
        return 0;
    }

    return ret;
}

PyObject *Py3dContactPoint_GetPosition(struct Py3dContactPoint *self, void *closure) {
    return Py_NewRef(self->position);
}

PyObject *Py3dContactPoint_GetNormal(struct Py3dContactPoint *self, void *closure) {
    return Py_NewRef(self->normal);
}

PyObject *Py3dContactPoint_GetDepth(struct Py3dContactPoint *self, void *closure) {
    return Py_NewRef(self->depth);
}
