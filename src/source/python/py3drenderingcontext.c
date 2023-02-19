#include "python/py3drenderingcontext.h"
#include "python/py3dtransform.h"
#include "python/python_util.h"
#include "logger.h"
#include "python/py3dgameobject.h"
#include "engine.h"
#include "util.h"

static PyObject *Py3dRenderingContext_Ctor = NULL;
static int Py3dRenderingContext_Init(struct Py3dRenderingContext *self, PyObject *args, PyObject *kwds);
static void Py3dRenderingContext_Dealloc(struct Py3dRenderingContext *self);
static PyMethodDef Py3dRenderingContext_Methods[] = {
        {NULL}
};

PyTypeObject Py3dRenderingContext_Type = {
        PyObject_HEAD_INIT(NULL)
        .tp_name = "py3dengine.RenderingContext",
        .tp_doc = "Represents the current rendering pass",
        .tp_basicsize = sizeof(struct Py3dRenderingContext),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_init = (initproc) Py3dRenderingContext_Init,
        .tp_methods = Py3dRenderingContext_Methods,
        .tp_dealloc = (destructor) Py3dRenderingContext_Dealloc,
        .tp_new = PyType_GenericNew
};

extern int PyInit_Py3dRenderingContext(PyObject *module) {
    if (PyType_Ready(&Py3dRenderingContext_Type) < 0) return 0;

    if (PyModule_AddObject(module, "RenderingContext", (PyObject *) &Py3dRenderingContext_Type) < 0) return 0;

    Py_INCREF(&Py3dRenderingContext_Type);

    return 1;
}

extern int Py3dRenderingContext_FindCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "RenderingContext") == 0) {
        critical_log("%s", "[RenderingContext]: Py3dRenderingContext has not been initialized properly");

        return 0;
    }

    Py3dRenderingContext_Ctor = PyObject_GetAttrString(module, "RenderingContext");

    return 1;
}

extern void Py3dRenderingContext_FinalizeCtor() {
    Py_CLEAR(Py3dRenderingContext_Ctor);
}

extern struct Py3dRenderingContext *Py3dRenderingContext_New(struct Py3dGameObject *activeCamera) {
    if (activeCamera == NULL) {
        PyErr_SetString(PyExc_ValueError, "Rendering Context requires validate camera object to construct");
        return NULL;
    }

    if (Py3dRenderingContext_Ctor == NULL) {
        PyErr_SetString(PyExc_TypeError, "RenderingContext has not been initialized properly");
        return NULL;
    }

    PyObject *ctorRet = PyObject_CallOneArg(Py3dRenderingContext_Ctor, (PyObject *) activeCamera);
    if (ctorRet == NULL) return NULL;

    return (struct Py3dRenderingContext *) ctorRet;
}

extern int Py3dRenderingContext_Check(PyObject *obj) {
    int ret = PyObject_IsInstance(obj, (PyObject *) &Py3dRenderingContext_Type);
    if (ret == -1) {
        handleException();
        return 0;
    }

    return ret;
}

struct PerspectiveCamera {
    float fovXInDegrees;
    float nearPlaneDistance;
    float farPlaceDistance;
};

static int extractFloatFromComponent(struct Py3dComponent *component, const char *name, float *dst) {
    if (component == NULL || dst == NULL) return 0;

    PyObject *getAttrRet = PyObject_GetAttrString(((PyObject *) component), name);
    if (getAttrRet == NULL) {
        PyErr_Clear();
        return 0;
    }

    PyObject *floatRet = PyNumber_Float(getAttrRet);
    if (floatRet == NULL) {
        PyErr_Clear();
        Py_CLEAR(getAttrRet);
        return 0;
    }

    (*dst) = (float) PyFloat_AsDouble(floatRet);
    Py_CLEAR(floatRet);
    Py_CLEAR(getAttrRet);

    return 1;
}

static int extractCameraFromComponent(struct Py3dComponent *component, struct PerspectiveCamera *camera) {
    if (component == NULL || camera == NULL) return 0;

    float fov_x_in_degrees = 0.0f;
    if (!extractFloatFromComponent(component, "fov_x_in_degrees", &fov_x_in_degrees)) return 0;

    float nearPlaceDistance = 0.0f;
    if (!extractFloatFromComponent(component, "near_z", &nearPlaceDistance)) return 0;

    float farPlaneDistance = 0.0f;
    if (!extractFloatFromComponent(component, "far_z", &farPlaneDistance)) return 0;

    camera->fovXInDegrees = fov_x_in_degrees;
    camera->nearPlaneDistance = nearPlaceDistance;
    camera->farPlaceDistance = farPlaneDistance;

    return 1;
}

static int extractPerspectiveCameraFromGameObject(struct Py3dGameObject *go, struct PerspectiveCamera *camera) {
    if (go == NULL || camera == NULL) {
        PyErr_SetString(PyExc_ValueError, "Could not extract camera from active camera Game Object");
        return 0;
    }

    Py_ssize_t componentCount = Py3dGameObject_GetComponentCountInt(go);
    for (Py_ssize_t i = 0; i < componentCount; ++i) {
        PyObject *getComponentRet = Py3dGameObject_GetComponentByIndexInt(go, i);
        if (getComponentRet == NULL) return 0;
        struct Py3dComponent *curComponent = (struct Py3dComponent *) getComponentRet;

        int extractRet = extractCameraFromComponent(curComponent, camera);
        Py_CLEAR(curComponent);
        if (extractRet == 1) return 1;
    }

    PyErr_SetString(PyExc_ValueError, "Could not extract camera from active camera Game Object");
    return 0;
}

static void buildPerspectiveMatrix(
        float dst[16],
        struct PerspectiveCamera *camera,
        int renderTargetWidth,
        int renderTargetHeight
) {
    if (dst == NULL || camera == NULL) return;

    float aspectRatio = ((float) renderTargetWidth) / ((float) renderTargetHeight);
    float aspectRatioInv = ((float) renderTargetHeight) / ((float) renderTargetWidth);
    float fov_y_radians = DEG_TO_RAD(aspectRatioInv * camera->fovXInDegrees);
    float w = 1.0f / (aspectRatio * tanf(fov_y_radians / 2.0f));
    float h = 1.0f / (tanf(fov_y_radians / 2.0f));
    float near_z = camera->nearPlaneDistance;
    float far_z = camera->farPlaceDistance;

    Mat4Identity(dst);
    dst[0] = w;
    dst[5] = h;
    dst[10] = far_z / (far_z - near_z);
    dst[11] = 1.0f;
    dst[14] = (-1.0f * near_z * far_z) / (far_z - near_z);
}

static int Py3dRenderingContext_Init(struct Py3dRenderingContext *self, PyObject *args, PyObject *kwds) {
    memset(self->vpMtx, 0, 16 * sizeof(float));
    memset(self->cameraPositionW, 0, 3 * sizeof(float));

    PyObject *cameraArg = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dGameObject_Type, &cameraArg) != 1) return -1;
    struct Py3dGameObject *activeCamera = (struct Py3dGameObject *) cameraArg;

    PyObject *getTransformRet = Py3dGameObject_GetTransform(activeCamera, NULL);
    if (getTransformRet == NULL) return -1;
    struct Py3dTransform *transform = (struct Py3dTransform *) getTransformRet;

    struct PerspectiveCamera camera;
    if (extractPerspectiveCameraFromGameObject(activeCamera, &camera) != 1) return -1;

    // TODO: when rendering targets are invented get the dimensions from it
    // right now, the engine is the rendering target
    int width = 0, height = 0;
    getRenderingTargetDimensions(&width, &height);

    float pMtx[16] = {0.0f};
    buildPerspectiveMatrix(pMtx, &camera, width, height);
    Mat4Mult(self->vpMtx, getTransformViewMtx(transform), pMtx);
    Vec3Copy(self->cameraPositionW, transform->position);
    Py_CLEAR(transform);

    return 0;
}

static void Py3dRenderingContext_Dealloc(struct Py3dRenderingContext *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}
