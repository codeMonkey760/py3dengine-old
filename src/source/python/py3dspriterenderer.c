#include "python/py3dspriterenderer.h"
#include "python/py3drenderingcontext.h"
#include "python/py3dtransform.h"
#include "python/py3dgameobject.h"
#include "python/py3dresourcemanager.h"
#include "python/python_util.h"
#include "resources/sprite.h"
#include "resources/model.h"
#include "resources/shader.h"
#include "resources/texture.h"

#include "logger.h"
#include "util.h"

static PyObject *Py3dSpriteRenderer_Ctor = NULL;

static int Py3dSpriteRenderer_Init(struct Py3dSpriteRenderer *self, PyObject *args, PyObject *kwds);
static void Py3dSpriteRenderer_Dealloc(struct Py3dSpriteRenderer *self);

static struct Py3dTransform *getTransform(struct Py3dSpriteRenderer *self) {
    PyObject *owner = Py3dComponent_GetOwner((struct Py3dComponent *) self, NULL);
    if (owner == NULL) {
        return NULL;
    } else if (Py_IsNone(owner)) {
        Py_CLEAR(owner);
        PyErr_SetString(PyExc_ValueError, "Cannot render a component that is detached from scene graph");
        return NULL;
    }

    PyObject *transform = Py3dGameObject_GetTransform((struct Py3dGameObject *) owner, NULL);
    Py_CLEAR(owner);
    if (transform == NULL) {
        return NULL;
    } else if (Py_IsNone(transform)) {
        Py_CLEAR(transform);
        PyErr_SetString(PyExc_ValueError, "Cannot render a component who's parent does not have a transform");
        return NULL;
    }

    return (struct Py3dTransform *) transform;
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

static PyMethodDef Py3dSpriteRenderer_Methods[] = {
    {"render", (PyCFunction) Py3dSpriteRenderer_Render, METH_VARARGS, "Render function for SpriteRendererComponent"},
    {"parse", (PyCFunction) Py3dSpriteRenderer_Parse, METH_VARARGS, "Parse function for SpriteRendererComponent"},
    {NULL}
};

PyTypeObject Py3dSpriteRenderer_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py3dengine.SpriteRendererComponent",
    .tp_doc = "Renders a sprite",
    .tp_basicsize = sizeof(struct Py3dSpriteRenderer),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) Py3dSpriteRenderer_Init,
    .tp_methods = Py3dSpriteRenderer_Methods,
    .tp_dealloc = (destructor) Py3dSpriteRenderer_Dealloc,
};

extern int PyInit_Py3dSpriteRenderer(PyObject *module) {
    Py3dSpriteRenderer_Type.tp_base = &Py3dComponent_Type;
    if (PyType_Ready(&Py3dSpriteRenderer_Type) < 0) return 0;

    if (PyModule_AddObject(module, "SpriteRendererComponent", (PyObject *) &Py3dSpriteRenderer_Type) < 0) return 0;

    Py_INCREF(&Py3dSpriteRenderer_Type);

    return 1;
}

extern int Py3dSpriteRenderer_FindCtor(PyObject *module) {
    Py3dSpriteRenderer_Ctor = PyObject_GetAttrString(module, "SpriteRendererComponent");
    if (Py3dSpriteRenderer_Ctor == NULL || PyCallable_Check(Py3dSpriteRenderer_Ctor) != 1) {
        critical_log("%s","[SpriteRendererComponent]: Unable to find initialize SpriteRendererComponent builtin");
        handleException();
        Py_CLEAR(Py3dSpriteRenderer_Ctor);
        return 0;
    }

    return 1;
}

extern void Py3dSpriteRenderer_FinalizeCtor() {
    Py_CLEAR(Py3dSpriteRenderer_Ctor);
}

int Py3dSpriteRenderer_Check(PyObject *obj) {
    int ret = PyObject_IsInstance(obj, (PyObject *) &Py3dSpriteRenderer_Type);
    if (ret == -1) {
        handleException();
        return 0;
    }

    return ret;
}

struct Py3dSpriteRenderer *Py3dSpriteRenderer_New() {
    if (Py3dSpriteRenderer_Ctor == NULL) return NULL;

    PyObject *newObj = PyObject_CallNoArgs(Py3dSpriteRenderer_Ctor);
    if (newObj == NULL || Py3dSpriteRenderer_Check(newObj) != 1) {
        critical_log("%s", "[SpriteRenderComponent]: New allocation failed");
        handleException();
        Py_CLEAR(newObj);
        return NULL;
    }

    return (struct Py3dSpriteRenderer *) newObj;
}

PyObject *Py3dSpriteRenderer_Render(struct Py3dSpriteRenderer *self, PyObject *args, PyObject *kwds) {
    if (self->sprite == NULL || self->quad == NULL || self->shader == NULL) {
        PyErr_SetString(PyExc_ValueError, "SpriteRendererComponent is not correctly configured");
        return NULL;
    }

    struct Py3dRenderingContext *rc = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dRenderingContext_Type, &rc) != 1) return NULL;

    struct Py3dTransform *transform = getTransform(self);
    if (transform == NULL) return NULL;

    enableShader(self->shader);

    float mixColor[3] = {1.0f, 1.0f, 1.0f};
    setShaderFloatArrayUniform(self->shader, "gMixColor", mixColor, 3);
    setShaderTextureUniform(self->shader, "gSprite", getSpriteSheet(self->sprite));

    float wvpMtx[16] = {0.0f};
    Mat4Identity(wvpMtx);
    Mat4Mult(wvpMtx, getTransformWorldMtx(transform), rc->vpMtx);
    Py_CLEAR(transform);
    setShaderMatrixUniform(self->shader, "gWVPMtx", wvpMtx, 4);

    float texMtx[9] = {0.0f};
    calcSpriteTextureMtx(self->sprite, texMtx);
    setShaderMatrixUniform(self->shader, "gTexMtx", texMtx, 3);

    bindModel(self->quad);
    renderModel(self->quad);
    unbindModel(self->quad);

    disableShader(self->shader);

    Py_RETURN_NONE;
}

PyObject *Py3dSpriteRenderer_Parse(struct Py3dSpriteRenderer *self, PyObject *args, PyObject *kwds) {
    PyObject *superParseRet = Py3dComponent_Parse((struct Py3dComponent *) self, args, kwds);
    if (superParseRet == NULL) return NULL;
    Py_CLEAR(superParseRet);

    PyObject *parseDataDict = NULL;
    struct Py3dResourceManager *py3dResourceManager = NULL;
    if (PyArg_ParseTuple(args, "O!O!", &PyDict_Type, &parseDataDict, &Py3dResourceManager_Type, &py3dResourceManager) != 1) return NULL;

    struct BaseResource *curRes = NULL;
    curRes = lookupResource("sprite", parseDataDict, py3dResourceManager);
    if (curRes == NULL) return NULL;
    if (!isResourceTypeSprite(curRes)) {
        PyErr_SetString(PyExc_ValueError, "Resource is not sprite");
        return NULL;
    } else {
        self->sprite = (struct Sprite *) curRes;
        curRes = NULL;
    }

    curRes = Py3dResourceManager_GetResource(py3dResourceManager, "QuadModelBuiltIn");
    if (!isResourceTypeModel(curRes)) {
        PyErr_SetString(PyExc_ValueError, "Could not find Quad Model Built In resource");
        return NULL;
    }
    self->quad = (struct Model *) curRes;
    curRes = NULL;

    curRes = Py3dResourceManager_GetResource(py3dResourceManager, "SpriteShaderBuiltIn");
    if (!isResourceTypeShader(curRes)) {
        PyErr_SetString(PyExc_ValueError, "Could not find Sprite Shader Built In resource");
        return NULL;
    }
    self->shader = (struct Shader *) curRes;
    curRes = NULL;

    Py_RETURN_NONE;
}

static int Py3dSpriteRenderer_Init(struct Py3dSpriteRenderer *self, PyObject *args, PyObject *kwds) {
    if (Py3dComponent_Type.tp_init((PyObject *) self, args, kwds) == -1) return -1;

    self->sprite = NULL;
    self->quad = NULL;
    self->shader = NULL;

    return 0;
}

static void Py3dSpriteRenderer_Dealloc(struct Py3dSpriteRenderer *self) {
    self->sprite = NULL;
    self->quad = NULL;
    self->shader = NULL;
    Py3dComponent_Dealloc((struct Py3dComponent *) self);
}
