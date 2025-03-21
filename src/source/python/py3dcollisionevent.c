#include "python/py3dcollisionevent.h"
#include "python/python_util.h"
#include "logger.h"

static PyObject *Py3dCollisionEvent_Ctor = NULL;

static int Py3dCollisionEvent_Init(struct Py3dCollisionEvent *self, PyObject *args, PyObject *kwds) {
    self->rigidBody1 = Py_NewRef(Py_None);
    self->rigidBody2 = Py_NewRef(Py_None);
    self->contactsTuple = Py_NewRef(Py_None);

    return 0;
}

static void Py3dCollisionEvent_Dealloc(struct Py3dCollisionEvent *self) {
    Py_CLEAR(self->rigidBody1);
    Py_CLEAR(self->rigidBody2);
    Py_CLEAR(self->contactsTuple);
}

static PyGetSetDef Py3dCollisionEvent_GetSet[] = {
    {"rigidBody1", (getter) Py3dCollisionEvent_GetRigidBody1, NULL, "Retrieve the first RigidBodyComponent involved in the collision"},
    {"rigidBody2", (getter) Py3dCollisionEvent_GetRigidBody2, NULL, "Retrieve the second RigidBodyComponent involved in the collision"},
    {"contacts", (getter) Py3dCollisionEvent_GetContacts, NULL, "Retrieve a tuple containing the contact points"},
    {NULL}
};

PyTypeObject Py3dCollisionEvent_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py3dengineEXT.CollisionEvent",
    .tp_doc = "Represents a collision event",
    .tp_basicsize = sizeof(struct Py3dCollisionEvent),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) Py3dCollisionEvent_Init,
    .tp_new = PyType_GenericNew,
    .tp_dealloc = (destructor) Py3dCollisionEvent_Dealloc,
    .tp_getset = Py3dCollisionEvent_GetSet
};

int PyInit_Py3dCollisionEvent(PyObject *module) {
    if (PyType_Ready(&Py3dCollisionEvent_Type) < 0) return 0;

    if (PyModule_AddObject(module, "CollisionEvent", (PyObject *) &Py3dCollisionEvent_Type) < 0) return 0;

    Py_INCREF(&Py3dCollisionEvent_Type);

    return 1;
}

int Py3dCollisionEvent_FindCtor(PyObject *module) {
    Py3dCollisionEvent_Ctor = PyObject_GetAttrString(module, "CollisionEvent");
    if (Py3dCollisionEvent_Ctor == NULL) {
        critical_log("%s", "[Python]: Py3dCollisionEvent has not been initialized properly");
        handleException();

        return 0;
    }

    return 1;
}

void Py3dCollisionEvent_FinalizeCtor() {
    Py_CLEAR(Py3dCollisionEvent_Ctor);
}

struct Py3dCollisionEvent *Py3dCollisionEvent_New() {
    if (Py3dCollisionEvent_Ctor == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Py3dCollisionEvent not initialized");
        return NULL;
    }

    struct Py3dCollisionEvent *py3dCollisionEvent = (struct Py3dCollisionEvent *) PyObject_CallNoArgs(Py3dCollisionEvent_Ctor);
    if (py3dCollisionEvent == NULL) {
        critical_log("%s", "[Python]: Failed to allocate Py3dCollisionEvent");
        return NULL;
    }

    if (!Py3dCollisionEvent_Check((PyObject *) py3dCollisionEvent)) {
        PyErr_SetString(PyExc_AssertionError, "Py3dCollisionEvent ctor did not return a CollisionEvent");
        Py_CLEAR(py3dCollisionEvent);
        return NULL;
    }

    return py3dCollisionEvent;
}

int Py3dCollisionEvent_Check(PyObject *obj) {
    int ret = PyObject_IsInstance(obj, (PyObject *) &Py3dCollisionEvent_Type);
    if (ret == -1) {
        handleException();
        return 0;
    }

    return ret;
}

PyObject *Py3dCollisionEvent_GetRigidBody1(struct Py3dCollisionEvent *self, void *closure) {
    return Py_NewRef(self->rigidBody1);
}

PyObject *Py3dCollisionEvent_GetRigidBody2(struct Py3dCollisionEvent *self, void *closure) {
    return Py_NewRef(self->rigidBody2);
}

PyObject *Py3dCollisionEvent_GetContacts(struct Py3dCollisionEvent *self, void *closure) {
    return Py_NewRef(self->contactsTuple);
}
