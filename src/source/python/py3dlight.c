#include "python/py3dlight.h"

#include "util.h"

#include "python/py3dscene.h"
#include "math/vector3.h"

#include "python/py3dcomponent.h"
#include "logger.h"
#include "python/python_util.h"
#include "lights.h"

static PyObject *Py3dLight_Ctor = NULL;

struct Py3dLight {
    struct Py3dComponent base;
    int lightType;
    float diffuse[3];
    float specular[3];
    float ambient[3];
    float intensity;
    float attenuation[3];
};

static int Py3dLight_Init(struct Py3dLight *self, PyObject *args, PyObject *kwds) {
    if (Py3dComponent_Type.tp_init((PyObject *) self, args, kwds) == -1) return -1;

    self->lightType = LIGHT_TYPE_UNKNOWN;
    Vec3Fill(self->diffuse, 0.0f);
    Vec3Fill(self->specular, 0.0f);
    Vec3Fill(self->ambient, 0.0f);
    self->intensity = 0.0f;
    Vec3Fill(self->attenuation, 0.0f);

    return 0;
}

static PyMethodDef Py3dLight_Methods[] = {
    {"parse", (PyCFunction) Py3dLight_Parse, METH_VARARGS, "Handle parse messages"},
    {"attach", (PyCFunction) Py3dLight_Attach, METH_VARARGS, "Handle attach messages"},
    {"detach", (PyCFunction) Py3dLight_Detach, METH_VARARGS, "Handle detach messages"},
    {"set_light_type", (PyCFunction) Py3dLight_SetType, METH_VARARGS, "Set light type"},
    {"set_diffuse_color", (PyCFunction) Py3dLight_SetDiffuseColor, METH_VARARGS, "Set diffuse color"},
    {"set_specular_color", (PyCFunction) Py3dLight_SetSpecularColor, METH_VARARGS, "Set specular color"},
    {"set_ambient_color", (PyCFunction) Py3dLight_SetAmbientColor, METH_VARARGS, "Set ambient color"},
    {"set_intensity", (PyCFunction) Py3dLight_SetIntensity, METH_VARARGS, "Set intensity"},
    {"set_attenuation", (PyCFunction) Py3dLight_SetAttenuation, METH_VARARGS, "Set attenuation params"},
    {NULL}
};

static void Py3dLight_Dealloc(struct Py3dModelRenderer *self) {
    // TODO: its not particularly clear if I should do this
    Py3dComponent_Dealloc((struct Py3dComponent *) self);

    //Py_TYPE(self)->tp_free((PyObject *) self);
}

PyTypeObject Py3dLight_Type = {
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

void Py3dLight_GetType(struct Py3dLight *self, int *dst) {
    if (self == NULL || dst == NULL) return;

    *dst = self->lightType;
}

void Py3dLight_GetDiffuse(struct Py3dLight *self, float dst[3]) {
    if (self == NULL || dst == NULL) return;

    Vec3Copy(dst, self->diffuse);
}

void Py3dLight_GetSpecular(struct Py3dLight *self, float dst[3]) {
    if (self == NULL || dst == NULL) return;

    Vec3Copy(dst, self->specular);
}

void Py3dLight_GetAmbient(struct Py3dLight *self, float dst[3]) {
    if (self == NULL || dst == NULL) return;

    Vec3Copy(dst, self->ambient);
}

void Py3dLight_GetIntensity(struct Py3dLight *self, float *dst) {
    if (self == NULL || dst == NULL) return;

    *dst = self->intensity;
}

void Py3dLight_GetAttenuation(struct Py3dLight *self, float dst[3]) {
    if (self == NULL || dst == NULL) return;

    Vec3Copy(dst, self->attenuation);
}

PyObject *Py3dLight_Parse(struct Py3dLight *self, PyObject *args, PyObject *kwds) {
    PyObject *superParseRet = Py3dComponent_Parse((struct Py3dComponent *) self, args, kwds);
    if (superParseRet == NULL) return NULL;
    Py_CLEAR(superParseRet);

    const char *compName = "LightComponent";

    PyObject *parseDataDict = NULL, *py3dResourceManager = NULL;
    if (PyArg_ParseTuple(args, "O!O", &PyDict_Type, &parseDataDict, &py3dResourceManager) != 1) return NULL;

    if (!Py3d_GetIntParseData(parseDataDict, "lightType", &self->lightType, compName)) return NULL;
    if (!Py3d_GetVector3ParseData(parseDataDict, "diffuse", self->diffuse, compName)) return NULL;
    if (!Py3d_GetVector3ParseData(parseDataDict, "specular", self->specular, compName)) return NULL;
    if (!Py3d_GetVector3ParseData(parseDataDict, "ambient", self->ambient, compName)) return NULL;
    if (!Py3d_GetFloatParseData(parseDataDict, "intensity", &self->intensity, compName)) return NULL;
    if (!Py3d_GetVector3ParseData(parseDataDict, "attenuation", self->attenuation, compName)) return NULL;

    Py_RETURN_NONE;
}

PyObject *Py3dLight_Attach(struct Py3dLight *self, PyObject *args, PyObject *kwds) {
    if (self == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Python function called with self set to NULL");
        return NULL;
    }

    struct Py3dScene *scene = Py3d_GetSceneForComponent((struct Py3dComponent *) self);
    if (scene == NULL) return NULL;

    const int result = Py3dScene_RegisterLight(scene, self);
    Py_CLEAR(scene);

    if (!result) {
        PyErr_SetString(PyExc_AssertionError, "Could not register light with containing scene");
        return NULL;
    }

    Py_RETURN_NONE;
}

PyObject *Py3dLight_Detach(struct Py3dLight *self, PyObject *args, PyObject *kwds) {
    if (self == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Python function called with self set to NULL");
        return NULL;
    }

    struct Py3dScene *scene = Py3d_GetSceneForComponent((struct Py3dComponent *) self);
    if (scene == NULL) return NULL;

    const int result = Py3dScene_UnRegisterLight(scene, self);
    Py_CLEAR(scene);

    if (!result) {
        PyErr_SetString(PyExc_AssertionError, "Could not unregister light with containing scene");
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject *setFloatArrayValue(float dst[3], PyObject *args) {
    struct Py3dVector3 *newColorAsVec3 = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dVector3_Type, &newColorAsVec3)) {
        // TODO: implement this
        PyErr_SetString(PyExc_NotImplementedError, "Setting value from Vector3 not yet implemented");
        return NULL;
    }
    PyErr_Clear();

    float components[3] = {0.0f};

    if (PyArg_ParseTuple(args, "(fff)", &components[0], &components[1], &components[2])) {
        Vec3Copy(dst, components);
        Py_RETURN_NONE;
    }
    PyErr_Clear();

    if (PyArg_ParseTuple(args, "fff", &components[0], &components[1], &components[2])) {
        // TODO: implement this
        PyErr_SetString(PyExc_NotImplementedError, "Setting value from individual floats not yet implemented");
        return NULL;
    }
    PyErr_Clear();

    PyErr_SetString(PyExc_ValueError, "Function requires Vector3 a sequence of 3 floats or 3 individual floats");
    return NULL;
}

PyObject *Py3dLight_SetType(struct Py3dLight *self, PyObject *args, PyObject *kwds) {
    if (self == NULL || args == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Python function called with invalid params");
        return NULL;
    }

    int newType = -1;
    if (!PyArg_ParseTuple(args, "i", &newType)) return NULL;

    self->lightType = newType; //TODO: since this is an enum, add additional validation

    Py_RETURN_NONE;
}

PyObject *Py3dLight_SetDiffuseColor(struct Py3dLight *self, PyObject *args, PyObject *kwds) {
    if (self == NULL || args == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Python function called with invalid params");
        return NULL;
    }

    return setFloatArrayValue(self->diffuse, args);
}

PyObject *Py3dLight_SetSpecularColor(struct Py3dLight *self, PyObject *args, PyObject *kwds) {
    if (self == NULL || args == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Python function called with invalid params");
        return NULL;
    }

    return setFloatArrayValue(self->specular, args);
}

PyObject *Py3dLight_SetAmbientColor(struct Py3dLight *self, PyObject *args, PyObject *kwds) {
    if (self == NULL || args == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Python function called with invalid params");
        return NULL;
    }

    return setFloatArrayValue(self->ambient, args);
}

PyObject *Py3dLight_SetIntensity(struct Py3dLight *self, PyObject *args, PyObject *kwds) {
    if (self == NULL || args == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Python function called with invalid params");
        return NULL;
    }

    float newIntensity = 0.0f;
    if (!PyArg_ParseTuple(args, "f", &newIntensity)) return NULL;

    self->intensity = newIntensity;

    Py_RETURN_NONE;
}

PyObject *Py3dLight_SetAttenuation(struct Py3dLight *self, PyObject *args, PyObject *kwds) {
    if (self == NULL || args == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Python function called with invalid params");
        return NULL;
    }

    return setFloatArrayValue(self->attenuation, args);
}