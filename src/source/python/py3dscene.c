#include "python/py3dscene.h"
#include <structmember.h>

#include "logger.h"
#include "python/python_util.h"

struct Py3dScene {
    PyObject_HEAD
    int enabled;
    int visible;
    PyObject *sceneGraph;
    PyObject *activeCamera;
};

static PyObject *py3dSceneCtor = NULL;

static int Py3dScene_Traverse(struct Py3dScene *self, visitproc visit, void *arg) {
    Py_VISIT(self->sceneGraph);
    Py_VISIT(self->activeCamera);

    return 0;
}

static int Py3dScene_Clear(struct Py3dScene *self) {
    Py_CLEAR(self->sceneGraph);
    Py_CLEAR(self->activeCamera);

    return 0;
}

static void Py3dScene_Dealloc(struct Py3dScene *self) {
    trace_log("%s", "[Scene]: Deallocating Scene");

    Py3dScene_Clear(self);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dScene_Init(struct Py3dScene *self, PyObject *args, PyObject *kwds) {
    trace_log("%s", "[Scene]: Initializing Scene");

    self->enabled = 1;
    self->visible = 1;
    self->sceneGraph = Py_NewRef(Py_None);
    self->activeCamera = Py_NewRef(Py_None);

    return 0;
}

PyMethodDef Py3dScene_Methods[] = {
    {NULL}
};

PyTypeObject Py3dScene_Type = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "py3dengine.Scene",
    .tp_basicsize = sizeof(struct Py3dScene),
    .tp_dealloc = (destructor) Py3dScene_Dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,
    .tp_doc = "Class for interacting with a scene graph",
    .tp_methods = Py3dScene_Methods,
    .tp_init = (initproc) Py3dScene_Init,
    .tp_new = PyType_GenericNew,
    .tp_traverse = (traverseproc) Py3dScene_Traverse,
    .tp_clear = (inquiry) Py3dScene_Clear
};

int PyInit_Py3dScene(PyObject *module) {
    if (PyType_Ready(&Py3dScene_Type) == -1) return 0;

    if (PyModule_AddObject(module, "Scene", (PyObject *) &Py3dScene_Type) == -1) return 0;

    Py_INCREF(&Py3dScene_Type);

    return 1;
}

int Py3dScene_FindCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "Scene") == 0) {
        critical_log("%s", "[Python]: Py3dScene has not been initialized properly");

        return 0;
    }

    py3dSceneCtor = PyObject_GetAttrString(module, "Scene");

    return 1;
}

void Py3dScene_FinalizeCtor() {
    Py_CLEAR(py3dSceneCtor);
}

int Py3dScene_Check(PyObject *obj) {
    if (obj == NULL) return 0;

    int ret = PyObject_IsInstance(obj, (PyObject *) &Py3dScene_Type);
    if (ret == -1) {
        handleException();
        return 0;
    }

    return ret;
}

struct Py3dScene *Py3dScene_New() {
    if (py3dSceneCtor == NULL) {
        critical_log("%s", "[Python]: Py3dScene has not been initialized properly");

        return NULL;
    }

    PyObject *args = PyTuple_New(0);
    PyObject *py3dScene = PyObject_Call(py3dScene, args, NULL);
    Py_CLEAR(args);
    if (py3dScene == NULL) {
        critical_log("%s", "[Python]: Failed to create Scene");
        handleException();

        return NULL;
    }

    if (!Py3dScene_Check(py3dScene)) {
        critical_log("%s", "[Python]: Scene ctor did not return scene");

        Py_CLEAR(py3dScene);
    }

    return (struct Py3dScene *) py3dScene;
}
