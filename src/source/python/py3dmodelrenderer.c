#include <glad/gl.h>

#include "python/py3dmodelrenderer.h"
#include "lights.h"
#include "logger.h"
#include "python/python_util.h"
#include "python/py3drenderingcontext.h"
#include "python/py3dgameobject.h"
#include "python/py3dresourcemanager.h"
#include "resources/shader.h"
#include "resources/material.h"
#include "resources/model.h"
#include "util.h"
#include "python/py3dscene.h"

// TODO: remove all of this hardcoded stuff once we have shader definition file parsing implemented
#define SHADER_PARAM_TYPE_INT 0
#define SHADER_PARAM_TYPE_FLOAT 1

struct ShaderParam {
    int type;
    const char *structName;
    const char *paramName;
    const size_t offset;
    int paramSize;
} shader_params[9] = {
    {
        SHADER_PARAM_TYPE_INT,
        "gLights",
        "used",
        0,
        1,
    },
    {
        SHADER_PARAM_TYPE_INT,
        "gLights",
        "enabled",
        0 + sizeof(GLint),
        1,
    },
    {
        SHADER_PARAM_TYPE_INT,
        "gLights",
        "lightType",
        0 + sizeof(GLint) + sizeof(GLint),
        1,
    },
    {
        SHADER_PARAM_TYPE_FLOAT,
        "gLights",
        "position",
        0 + sizeof(GLint) + sizeof(GLint) + sizeof(GLint),
        3,
    },
    {
        SHADER_PARAM_TYPE_FLOAT,
        "gLights",
        "diffuse",
        0 + sizeof(GLint) + sizeof(GLint) + sizeof(GLint) + (sizeof(float) * 3),
        3,
    },
    {
        SHADER_PARAM_TYPE_FLOAT,
        "gLights",
        "specular",
        0 + sizeof(GLint) + sizeof(GLint) + sizeof(GLint) + (sizeof(float) * 3) + (sizeof(float) * 3),
        3,
    },
    {
        SHADER_PARAM_TYPE_FLOAT,
        "gLights",
        "ambient",
        0 + sizeof(GLint) + sizeof(GLint) + sizeof(GLint) + (sizeof(float) * 3) + (sizeof(float) * 3) + (sizeof(float) * 3),
        3,
    },
    {
        SHADER_PARAM_TYPE_FLOAT,
        "gLights",
        "intensity",
        0 + sizeof(GLint) + sizeof(GLint) + sizeof(GLint) + (sizeof(float) * 3) + (sizeof(float) * 3) + (sizeof(float) * 3) + (sizeof(float) * 3),
        1,
    },
    {
        SHADER_PARAM_TYPE_FLOAT,
        "gLights",
        "attenuation",
        0 + sizeof(GLint) + sizeof(GLint) + sizeof(GLint) + (sizeof(float) * 3) + (sizeof(float) * 3) + (sizeof(float) * 3) + (sizeof(float) * 3) + sizeof(float),
        3,
    },
};

static void setShaderParam(struct Shader *shader, int curLight, int curParam, struct LightData *lightData) {
    char name_buffer[32+1] = {0};
    snprintf(name_buffer, 32, "%s[%d].%s", shader_params[curParam].structName, curLight, shader_params[curParam].paramName);

    const size_t lightOffset = sizeof(struct LightData) * curLight;
    const size_t paramOffset = shader_params[curParam].offset;
    const void *dataSrc = ((void *) lightData) + lightOffset + paramOffset;

    if (shader_params[curParam].type == SHADER_PARAM_TYPE_INT) {
        setShaderIntUniform(shader, name_buffer, dataSrc, shader_params[curParam].paramSize);
    } else if (shader_params[curParam].type == SHADER_PARAM_TYPE_FLOAT) {
        setShaderFloatArrayUniform(shader, name_buffer, dataSrc, shader_params[curParam].paramSize);
    }
}

static PyObject *Py3dModelRenderer_Ctor = NULL;

static PyObject *Py3dModelRenderer_Render(struct Py3dModelRenderer *self, PyObject *args, PyObject *kwds) {
    if (self->shader == NULL || self->model == NULL || self->material == NULL) {
        PyErr_SetString(PyExc_ValueError, "ModelRendererComponent is not correctly configured");
        return NULL;
    }

    struct Py3dRenderingContext *rc = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dRenderingContext_Type, &rc) != 1) return NULL;

    struct Py3dGameObject *owner = Py3d_GetOwnerForComponent((struct Py3dComponent *) self);
    if (owner == NULL) return NULL;

    struct Py3dScene *scene = Py3d_GetSceneForGameObject(owner);
    if (scene == NULL) return NULL;

    enableShader(self->shader);
    setShaderFloatArrayUniform(self->shader, "gCamPos", Py3dRenderingContext_GetCameraPosW(rc), 3);
    setShaderMatrixUniform(self->shader, "gWMtx", Py3dGameObject_GetWorldMatrix(owner), 4);
    setShaderMatrixUniform(self->shader, "gWITMtx", Py3dGameObject_GetWITMatrix(owner), 4);
    setShaderTextureUniform(self->shader, "gMaterial.diffuse", self->material->_diffuseMap);
    setShaderFloatArrayUniform(self->shader, "gMaterial.ambient", getMaterialAmbientColor(self->material), 3);
    setShaderFloatArrayUniform(self->shader, "gMaterial.specular", getMaterialSpecularColor(self->material), 3);
    setShaderFloatArrayUniform(self->shader, "gMaterial.specPower", getMaterialSpecPower(self->material), 1);

    struct LightData *lightData = NULL;
    size_t numLights = 0;
    Py3dScene_GetDynamicLightData(scene, &lightData, &numLights);

    for (int curLight = 0; curLight < numLights; ++curLight) {
        for (int curParam = 0; curParam < 9; ++curParam) {
            setShaderParam(self->shader, curLight, curParam, lightData);
        }
    }

    float wvpMtx[16] = {0.0f};
    Mat4Identity(wvpMtx);
    Mat4Mult(wvpMtx, Py3dGameObject_GetWorldMatrix(owner), Py3dRenderingContext_GetCameraVPMtx(rc));
    setShaderMatrixUniform(self->shader, "gWVPMtx", wvpMtx, 4);

    bindModel(self->model);
    renderModel(self->model);
    unbindModel(self->model);

    disableShader(self->shader);

    Py_CLEAR(scene);
    Py_CLEAR(owner);

    Py_RETURN_NONE;
}

static struct BaseResource *lookupResource(const char *name, PyObject *parseDataDict, struct Py3dResourceManager *rm) {
    PyObject *curAttr = NULL;
    curAttr = PyDict_GetItemString(parseDataDict, name);
    if (curAttr == NULL) return NULL;
    // TODO: PyDict_GetItemString returns borrowed reference so this overwrite isnt a leak ... right?
    curAttr = PyObject_Str(curAttr);
    if (curAttr == NULL) return NULL;

    const char *modelName = PyUnicode_AsUTF8(curAttr);
    struct BaseResource *ret = Py3dResourceManager_GetResource(rm, modelName);
    Py_CLEAR(curAttr);

    if (ret == NULL) {
        PyErr_SetString(PyExc_ValueError, "Resource not imported");
        return NULL;
    }

    return ret;
}

static PyObject *Py3dModelRenderer_Parse(struct Py3dModelRenderer *self, PyObject *args, PyObject *kwds) {
    PyObject *superParseRet = Py3dComponent_Parse((struct Py3dComponent *) self, args, kwds);
    if (superParseRet == NULL) return NULL;
    Py_CLEAR(superParseRet);

    PyObject *parseDataDict = NULL;
    struct Py3dResourceManager *py3dResourceManager = NULL;
    if (PyArg_ParseTuple(args, "O!O!", &PyDict_Type, &parseDataDict, &Py3dResourceManager_Type, &py3dResourceManager) != 1) return NULL;

    struct BaseResource *curRes = NULL;

    curRes = lookupResource("model", parseDataDict, py3dResourceManager);
    if (curRes == NULL) return NULL;
    if (!isResourceTypeModel(curRes)) {
        PyErr_SetString(PyExc_ValueError, "Resource is not model");
        return NULL;
    } else {
        self->model = (struct Model *) curRes;
        curRes = NULL;
    }

    curRes = lookupResource("shader", parseDataDict, py3dResourceManager);
    if (curRes == NULL) return NULL;
    if (!isResourceTypeShader(curRes)) {
        PyErr_SetString(PyExc_ValueError, "Resource is not shader");
        return NULL;
    } else {
        self->shader = (struct Shader *) curRes;
        curRes = NULL;
    }

    curRes = lookupResource("material", parseDataDict, py3dResourceManager);
    if (curRes == NULL) return NULL;
    if (!isResourceTypeMaterial(curRes)) {
        PyErr_SetString(PyExc_ValueError, "Resource is not material");
        return NULL;
    } else {
        self->material = (struct Material *) curRes;
        curRes = NULL;
    }

    Py_RETURN_NONE;
}

static PyMethodDef Py3dModelRenderer_Methods[] = {
    {"render", (PyCFunction) Py3dModelRenderer_Render, METH_VARARGS, "Render function for ModelRendererComponent"},
    {"parse", (PyCFunction) Py3dModelRenderer_Parse, METH_VARARGS, "Parse function for ModelRendererComponent"},
    {NULL}
};

static void Py3dModelRenderer_Dealloc(struct Py3dModelRenderer *self) {
    // TODO: its not particularly clear if I should do this
    Py3dComponent_Dealloc((struct Py3dComponent *) self);

    //Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dModelRenderer_Init(struct Py3dModelRenderer *self, PyObject *args, PyObject *kwds) {
    if (Py3dComponent_Type.tp_init((PyObject *) self, args, kwds) == -1) return -1;

    self->material = NULL;
    self->model = NULL;
    self->shader = NULL;

    return 0;
}

static PyTypeObject Py3dModelRenderer_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py3dengineEXT.ModelRendererComponent",
    .tp_doc = "Renders a 3d model with a shader and a material",
    .tp_basicsize = sizeof(struct Py3dModelRenderer),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) Py3dModelRenderer_Init,
    .tp_new = PyType_GenericNew,
    .tp_methods = Py3dModelRenderer_Methods,
    .tp_dealloc = (destructor) Py3dModelRenderer_Dealloc,
};

int PyInit_Py3dModelRenderer(PyObject *module) {
    Py3dModelRenderer_Type.tp_base = &Py3dComponent_Type;
    if (PyType_Ready(&Py3dModelRenderer_Type) < 0) return false;

    if (PyModule_AddObject(module, "ModelRendererComponent", (PyObject *) &Py3dModelRenderer_Type) < 0) return false;

    Py_INCREF(&Py3dModelRenderer_Type);

    return true;
}

int Py3dModelRenderer_FindCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "ModelRendererComponent") == 0) {
        critical_log("%s", "[Python]: Py3dModelRenderer has not been initialized properly");

        return false;
    }

    Py3dModelRenderer_Ctor = PyObject_GetAttrString(module, "ModelRendererComponent");

    return true;
}

void Py3dModelRenderer_FinalizeCtor() {
    Py_CLEAR(Py3dModelRenderer_Ctor);
}

struct Py3dModelRenderer *Py3dModelRenderer_New() {
    if (Py3dModelRenderer_Ctor == NULL) {
        critical_log("%s", "[Python]: Py3dModelRenderer has not been initialized properly");

        return NULL;
    }

    struct Py3dModelRenderer *py3dModelRenderer = (struct Py3dModelRenderer *) PyObject_CallNoArgs(Py3dModelRenderer_Ctor);
    if (py3dModelRenderer == NULL) {
        critical_log("%s", "[Python]: Failed to allocate ModelRendererComponent in python interpreter");
        handleException();

        return NULL;
    }

    return py3dModelRenderer;
}

int Py3dModelRenderer_Check(PyObject *obj) {
    return PyObject_IsInstance(obj, (PyObject *) &Py3dModelRenderer_Type);
}
