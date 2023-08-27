#include "python/py3dtextrenderer.h"
#include "python/python_util.h"
#include "logger.h"
#include "python/py3dresourcemanager.h"
#include "python/py3drenderingcontext.h"
#include "resources/model.h"
#include "resources/shader.h"
#include "resources/texture.h"
#include "util.h"
#include "engine.h"

struct Py3dTextRenderer {
    struct Py3dComponent base;
    struct Model *quad;
    struct Shader *shader;
    struct Texture *char_map;
    PyObject *text;
    float color[4];
};

static PyMethodDef Py3dTextRenderer_Methods[] = {
    {"render", (PyCFunction) Py3dTextRenderer_Render, METH_VARARGS, "Render function for TextRendererComponent"},
    {"parse", (PyCFunction) Py3dTextRenderer_Parse, METH_VARARGS, "Parse function for TextRendererComponent"},
    {"set_text", (PyCFunction) Py3dTextRenderer_SetText, METH_VARARGS, "Set the text to render"},
    {"set_color", (PyCFunction) Py3dTextRenderer_SetColor, METH_VARARGS, "Set the text color"},
    {NULL}
};

static int Py3dTextRenderer_Init(struct Py3dTextRenderer *self, PyObject *args, PyObject *kwds);
static void Py3dTextRenderer_Dealloc(struct Py3dTextRenderer *self);

PyTypeObject Py3dTextRenderer_Type = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "py3dengine.TextRendererComponent",
    .tp_doc = "Renders text",
    .tp_basicsize = sizeof(struct Py3dTextRenderer),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) Py3dTextRenderer_Init,
    .tp_new = PyType_GenericNew,
    .tp_methods = Py3dTextRenderer_Methods,
    .tp_dealloc = (destructor) Py3dTextRenderer_Dealloc,
};

int PyInit_Py3dTextRenderer(PyObject *module) {
    Py3dTextRenderer_Type.tp_base = &Py3dComponent_Type;
    if (PyType_Ready(&Py3dTextRenderer_Type) < 0) return 0;

    if (PyModule_AddObject(module, "TextRendererComponent", (PyObject *) &Py3dTextRenderer_Type) < 0) return 0;

    Py_INCREF(&Py3dTextRenderer_Type);

    return 1;
}

int Py3dTextRenderer_Check(PyObject *obj) {
    int ret = PyObject_IsInstance(obj, (PyObject *) &Py3dTextRenderer_Type);
    if (ret == -1) {
        handleException();
        return 0;
    }

    return ret;
}

struct Py3dTextRenderer *Py3dTextRenderer_New() {
    PyObject *py3dengine = PyImport_ImportModule("py3dengine");
    if (py3dengine == NULL) {
        critical_log("[TextRendererComponent]: Failed to import py3dengine module");
        return NULL;
    }

    PyObject *ctor = PyObject_GetAttrString(py3dengine, "TextRendererComponent");
    Py_CLEAR(py3dengine);

    if (ctor == NULL) {
        critical_log("[TextRendererComponent]: Failed to find ctor in py3dengine module");
        return NULL;
    }

    PyObject *args = Py_BuildValue("()");
    PyObject *ret = PyObject_Call(ctor, args, NULL);

    Py_CLEAR(args);
    Py_CLEAR(ctor);

    if (ret == NULL) {
        critical_log("[TextRendererComponent]: Failed to instantiate new object");
        return NULL;
    } else if (!Py3dTextRenderer_Check(ret)) {
        critical_log("[TextRendererComponent]: Newly instantiated object failed type check");
        Py_CLEAR(ret);
        PyErr_SetString(PyExc_TypeError, "Newly instantiated object failed type check");
        return NULL;
    }

    return (struct Py3dTextRenderer *) ret;
}

static int Py3dTextRenderer_Init(struct Py3dTextRenderer *self, PyObject *args, PyObject *kwds) {
    if (Py3dComponent_Type.tp_init( (PyObject *) self, args, kwds) == -1) return -1;

    self->quad = NULL;
    self->shader = NULL;
    self->char_map = NULL;
    self->text = Py_NewRef(Py_None);
    self->color[0] = 0.0f;
    self->color[1] = 0.0f;
    self->color[2] = 0.0f;
    self->color[3] = 0.0f;

    return 0;
}

static void Py3dTextRenderer_Dealloc(struct Py3dTextRenderer *self) {
    self->quad = NULL;
    self->shader = NULL;
    self->char_map = NULL;
    Py_CLEAR(self->text);
    Py3dComponent_Dealloc((struct Py3dComponent *) self);
}

static void calcCharWVPMtx(float out[16], int row, int col, int font_width, int font_height) {
    int screen_width = 0, screen_height = 0;
    getRenderingTargetDimensions(&screen_width, &screen_height);

    float quad_width_in_pixels = ((float) screen_width) / 2.0f;
    float quad_height_in_pixels = ((float) screen_height) / 2.0f;
    float glyph_width_in_units = ((float) font_width) / quad_width_in_pixels;
    float glyph_height_in_units = ((float) font_height) / quad_height_in_pixels;

    float S[16];
    Mat4ScalingF(S, glyph_width_in_units, glyph_height_in_units, 1.0f);

    float T[16];
    float x = (((float) col) * glyph_width_in_units) + (glyph_width_in_units / 2.0f) - 1.0f;
    float y = (((float) row * -1) * glyph_height_in_units) - (glyph_height_in_units / 2.0f) + 1.0f;
    Mat4TranslationF(T, x, y, 0.0f);

    Mat4Mult(out, S, T);
}

static void calcCharTexMtx(float out[9], char c, int font_width, int font_height) {
    float glyph_width_in_units = ((float) font_width) / 1024.0f;
    int spaces = c - ' ';
    float offset = ((float) spaces) * glyph_width_in_units;

    out[0] = glyph_width_in_units;
    out[1] = 0.0f;
    out[2] = 0.0f;

    out[3] = 0.0f;
    out[4] = 1.0f; // TODO: this might be too tall
    out[5] = 0.0f;

    out[6] = offset;
    out[7] = 0.0f;
    out[8] = 1.0f;
}

static void advanceCursor(char c, int *col, int *row) {
    if (c == '\n') {
        (*col) = 0;
        (*row)++;
    } else {
        (*col)++;
    }
}

PyObject *Py3dTextRenderer_Render(struct Py3dTextRenderer *self, PyObject *args, PyObject *kwds) {
    if (self->quad == NULL || self->shader == NULL || self->char_map == NULL) {
        PyErr_SetString(PyExc_ValueError, "TextRendererComponent is not correctly configured");
        return NULL;
    }

    if (Py_IsNone(self->text)) {
        Py_RETURN_NONE;
    }

    struct Py3dRenderingContext *rc = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dRenderingContext_Type, &rc) != 1) return NULL;

    Py_ssize_t textLen = 0;
    const char *text = PyUnicode_AsUTF8AndSize(self->text, &textLen);

    enableShader(self->shader);

    setShaderFloatArrayUniform(self->shader, "gMixColor", self->color, 3);
    setShaderTextureUniform(self->shader, "gSprite", self->char_map);

    bindModel(self->quad);

    float wvpMtx[16];
    float texMtx[9];
    int col = 0, row = 0;

    for (Py_ssize_t i = 0; i < textLen; ++i) {
        const char curChar = text[i];

        if (isprint(curChar)) {
            calcCharWVPMtx(wvpMtx, row, col, 8, 16);
            setShaderMatrixUniform(self->shader, "gWVPMtx", wvpMtx, 4);

            calcCharTexMtx(texMtx, curChar, 8, 16);
            setShaderMatrixUniform(self->shader, "gTexMtx", texMtx, 3);

            renderModel(self->quad);
        }
        advanceCursor(curChar, &col, &row);
    }

    unbindModel(self->quad);

    disableShader(self->shader);

    Py_RETURN_NONE;
}

PyObject *Py3dTextRenderer_Parse(struct Py3dTextRenderer *self, PyObject *args, PyObject *kwds) {
    PyObject *superParseRet = Py3dComponent_Parse((struct Py3dComponent *) self, args, kwds);
    if (superParseRet == NULL) return NULL;
    Py_CLEAR(superParseRet);

    PyObject *parseDataDict = NULL;
    struct Py3dResourceManager *py3dResourceManager = NULL;
    if (PyArg_ParseTuple(args, "O!O!", &PyDict_Type, &parseDataDict, &Py3dResourceManager_Type, &py3dResourceManager) != 1) return NULL;

    struct BaseResource *curRes = NULL;
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

    curRes = Py3dResourceManager_GetResource(py3dResourceManager, "charmap");
    if (!isResourceTypeTexture(curRes)) {
        PyErr_SetString(PyExc_ValueError, "Could not find charmap");
        return NULL;
    }
    self->char_map = (struct Texture *) curRes;
    curRes = NULL;

    PyObject *text = PyDict_GetItemString(parseDataDict, "text"); // text is a borrowed ref
    if (text == NULL) return NULL;
    PyObject *setTextArgs = Py_BuildValue("(O)", text);
    PyObject *setTextRet = Py3dTextRenderer_SetText(self, setTextArgs, NULL);
    Py_CLEAR(setTextArgs);
    if (setTextRet == NULL) return NULL;
    Py_CLEAR(setTextRet);

    PyObject *color = PyDict_GetItemString(parseDataDict, "color");
    if (color == NULL) return NULL;
    PyObject *setColorArgs = PySequence_Tuple(color);
    PyObject *setColorRet = Py3dTextRenderer_SetColor(self, setColorArgs, NULL);
    Py_CLEAR(setColorArgs);
    if (setColorRet == NULL) return NULL;
    Py_CLEAR(setColorRet);

    Py_RETURN_NONE;
}

PyObject *Py3dTextRenderer_SetText(struct Py3dTextRenderer *self, PyObject *args, PyObject *kwds) {
    PyObject *textArg = NULL;
    if (PyArg_ParseTuple(args, "O", &textArg) != 1) return NULL;

    PyObject *textArgAsStr = PyObject_Str(textArg);
    if (textArgAsStr == NULL) return NULL;

    Py_CLEAR(self->text);
    self->text = textArgAsStr;

    Py_RETURN_NONE;
}

PyObject *Py3dTextRenderer_SetColor(struct Py3dTextRenderer *self, PyObject *args, PyObject *kwds) {
    float r, g, b, a;
    if (PyArg_ParseTuple(args, "ffff", &r, &g, &b, &a) != 1) return NULL;

    self->color[0] = r;
    self->color[1] = g;
    self->color[2] = b;
    self->color[3] = a;

    Py_RETURN_NONE;
}
