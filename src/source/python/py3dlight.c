#include "python/py3dlight.h"
#include "python/py3dcomponent.h"
#include "logger.h"
#include "python/python_util.h"
#include "lights.h"

static PyObject *Py3dLight_Ctor = NULL;

struct Py3dLight {
    struct Py3dComponent base;
    struct LightData data;
};

static int Py3dLight_Init(struct Py3dLight *self, PyObject *args, PyObject *kwds) {
    if (Py3dComponent_Type.tp_init((PyObject *) self, args, kwds) == -1) return -1;

    LightData_Init(&self->data);

    return 0;
}

static PyMethodDef Py3dLight_Methods[] = {
    {"parse", (PyCFunction) Py3dLight_Parse, METH_VARARGS, "Handle parse messages"},
    {NULL}
};

static void Py3dLight_Dealloc(struct Py3dModelRenderer *self) {
    // TODO: its not particularly clear if I should do this
    Py3dComponent_Dealloc((struct Py3dComponent *) self);

    //Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyTypeObject Py3dLight_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py3dengine.LightComponent",
    .tp_doc = "A dynamic light",
    .tp_basicsize = sizeof(struct Py3dLight),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) Py3dLight_Init,
    .tp_new = PyType_GenericNew,
    .tp_methods = Py3dLight_Methods,
    .tp_dealloc = (destructor) Py3dLight_Dealloc,
};

int PyInit_Py3dLight(PyObject *module) {
    Py3dLight_Type.tp_base = &Py3dComponent_Type;
    if (PyType_Ready(&Py3dLight_Type) < 0) return 0;

    if (PyModule_AddObject(module, "LightComponent", (PyObject *) &Py3dLight_Type) < 0) return 0;

    Py_INCREF(&Py3dLight_Type);

    return 1;
}

int Py3dLight_FindCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "LightComponent") == 0) {
        critical_log("%s", "[Python]: Py3dLight has not been initialized properly");

        return 0;
    }

    Py3dLight_Ctor = PyObject_GetAttrString(module, "LightComponent");

    return 1;
}

void Py3dLight_FinalizeCtor() {
    Py_CLEAR(Py3dLight_Ctor);
}

struct Py3dLight *Py3dLight_New() {
    if (Py3dLight_Ctor == NULL) {
        critical_log("%s", "[Python]: Py3dLight has not been initialized properly");

        return NULL;
    }

    struct Py3dLight *py3dLight = (struct Py3dLight *) PyObject_CallNoArgs(Py3dLight_Ctor);
    if (py3dLight == NULL) {
        critical_log("%s", "[Python]: Failed to allocate LightComponent in python interpreter");
        handleException();

        return NULL;
    }

    return py3dLight;
}

int Py3dLight_Check(PyObject *obj) {
    int ret = PyObject_IsInstance(obj, (PyObject *) &Py3dLight_Type);
    if (ret == -1) {
        handleException();
        ret = 0;
    }

    return ret;
}

PyObject *Py3dLight_Parse(struct Py3dLight *self, PyObject *args, PyObject *kwds) {
    PyObject *superParseRet = Py3dComponent_Parse((struct Py3dComponent *) self, args, kwds);
    if (superParseRet == NULL) return NULL;
    Py_CLEAR(superParseRet);

    const char *compName = "LightComponent";

    PyObject *parseDataDict = NULL, *py3dResourceManager = NULL;
    if (PyArg_ParseTuple(args, "O!O", &PyDict_Type, &parseDataDict, &py3dResourceManager) != 1) return NULL;

    if (!Py3d_GetBooleanParseData(parseDataDict, "enabled", &self->data.enabled, compName)) return NULL;
    if (!Py3d_GetIntParseData(parseDataDict, "type", &self->data.type, compName)) return NULL;
    if (!Py3d_GetVector3ParseData(parseDataDict, "diffuse", self->data.diffuse, compName)) return NULL;
    if (!Py3d_GetVector3ParseData(parseDataDict, "specular", self->data.specular, compName)) return NULL;
    if (!Py3d_GetVector3ParseData(parseDataDict, "ambient", self->data.ambient, compName)) return NULL;
    if (!Py3d_GetFloatParseData(parseDataDict, "intensity", &self->data.intensity, compName)) return NULL;
    if (!Py3d_GetVector3ParseData(parseDataDict, "attenuation", self->data.attenuation, compName)) return NULL;

    Py_RETURN_NONE;
}