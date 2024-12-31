#include "python/py3drenderingcontext.h"

#include <python/py3dcomponent.h>

#include "python/python_util.h"
#include "logger.h"
#include "python/py3dgameobject.h"
#include "python/py3dscene.h"
#include "engine.h"
#include "util.h"

struct PerspectiveCamera {
    float fovXInDegrees;
    float nearPlaneDistance;
    float farPlaceDistance;
    float vpMtx[16];
    float posW[3];
};

static void initPerspectiveCamera(struct PerspectiveCamera *camera) {
    if (camera == NULL) return;

    camera->fovXInDegrees = 0.0f;
    camera->nearPlaneDistance = 0.0f;
    camera->farPlaceDistance = 0.0f;
    memset(camera->vpMtx, 0, sizeof(float) * 16);
    memset(camera->posW, 0, sizeof(float) * 3);
}

struct Py3dRenderingContext {
    PyObject_HEAD
    struct Py3dScene *scene;
    struct PerspectiveCamera camera;
};

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

extern struct Py3dRenderingContext *Py3dRenderingContext_New(struct Py3dScene *scene) {
    if (scene == NULL) {
        PyErr_SetString(PyExc_ValueError, "Rendering Context requires scene to construct");
        return NULL;
    }

    if (Py3dRenderingContext_Ctor == NULL) {
        PyErr_SetString(PyExc_TypeError, "RenderingContext has not been initialized properly");
        return NULL;
    }

    PyObject *ctorRet = PyObject_CallOneArg(Py3dRenderingContext_Ctor, (PyObject *) scene);
    if (ctorRet == NULL) return NULL;

    if (!Py3dRenderingContext_Check(ctorRet)) {
        Py_CLEAR(ctorRet);
        PyErr_SetString(PyExc_TypeError, "RenderingContext ctor did not return a rendering context");
        return NULL;
    }

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

static int extractFloatFromComponent(struct Py3dComponent *component, const char *name, float *dst) {
    if (component == NULL || name == NULL || dst == NULL) return 0;

    PyObject *getAttrRet = PyObject_GetAttrString((PyObject *) component, name);
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

    *dst = (float) PyFloat_AsDouble(floatRet);
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
    if (go == NULL || camera == NULL) return 0;

    const Py_ssize_t componentCount = Py3dGameObject_GetComponentCountInt(go);
    for (Py_ssize_t i = 0; i < componentCount; ++i) {
        struct Py3dComponent *curComponent = NULL;

        PyObject *getComponentRet = Py3dGameObject_GetComponentByIndexInt(go, i);
        if (getComponentRet == NULL) {
            return 0;
        } else if (!Py3dComponent_Check(getComponentRet)) {
            Py_CLEAR(getComponentRet);
            return 0;
        } else {
            curComponent = (struct Py3dComponent *) getComponentRet;
            getComponentRet = NULL;
        }

        const int extractRet = extractCameraFromComponent(curComponent, camera);
        Py_CLEAR(curComponent);

        if (extractRet == 1) return 1;
    }

    return 0;
}

static void buildPerspectiveMatrix(
    float dst[16],
    const struct PerspectiveCamera *camera,
    const int renderTargetWidth,
    const int renderTargetHeight
) {
    if (dst == NULL || camera == NULL) return;

    const float aspectRatio = ((float) renderTargetWidth) / ((float) renderTargetHeight);
    const float aspectRatioInv = ((float) renderTargetHeight) / ((float) renderTargetWidth);
    const float fov_y_radians = DEG_TO_RAD(aspectRatioInv * camera->fovXInDegrees);
    const float w = 1.0f / (aspectRatio * tanf(fov_y_radians / 2.0f));
    const float h = 1.0f / (tanf(fov_y_radians / 2.0f));
    const float near_z = camera->nearPlaneDistance;
    const float far_z = camera->farPlaceDistance;

    Mat4Identity(dst);
    dst[0] = w;
    dst[5] = h;
    dst[10] = far_z / (far_z - near_z);
    dst[11] = 1.0f;
    dst[14] = (-1.0f * near_z * far_z) / (far_z - near_z);
    dst[15] = 0.0f;
}

static void setCamera(struct Py3dRenderingContext *self, struct Py3dGameObject *newCamera) {
    if (extractPerspectiveCameraFromGameObject(newCamera, &self->camera) != 1) return;

    // TODO: when rendering targets are invented get the dimensions from it
    // right now, the glfw window is the rendering target
    // fyi the rendering target will come from the camera
    int width = 0, height = 0;
    getRenderingTargetDimensions(&width, &height);

    float vMtx[16] = {0.0f};
    float pMtx[16] = {0.0f};
    Py3dGameObject_CalculateViewMatrix(newCamera, vMtx);
    buildPerspectiveMatrix(pMtx, &self->camera, width, height);
    Mat4Mult(self->camera.vpMtx, vMtx, pMtx);
    Vec3Copy(self->camera.posW, Py3dGameObject_GetPositionFA(newCamera));
}

static int Py3dRenderingContext_Init(struct Py3dRenderingContext *self, PyObject *args, PyObject *kwds) {
    self->scene = NULL;
    initPerspectiveCamera(&self->camera);

    struct Py3dScene *scene = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dScene_Type, &scene) != 1) return -1;
    self->scene = (struct Py3dScene *) Py_NewRef(scene);
    scene = NULL;

    struct Py3dGameObject *activeCamera = NULL;
    PyObject *getActiveCameraRet = Py3dScene_GetActiveCamera(self->scene);
    if (getActiveCameraRet == NULL) {
        return -1;
    } else if (!Py3dGameObject_Check(getActiveCameraRet)) {
        Py_CLEAR(getActiveCameraRet);
        error_log("[Py3dRenderingContext]: Scene without GameObject as active camera discovered");
        PyErr_SetString(PyExc_TypeError, "Scene without GameObject as active camera discovered");
        return -1;
    } else {
        activeCamera = (struct Py3dGameObject *) getActiveCameraRet;
        getActiveCameraRet = NULL;
    }

    setCamera(self, activeCamera);
    Py_CLEAR(activeCamera);

    return 0;
}

static void Py3dRenderingContext_Dealloc(struct Py3dRenderingContext *self) {
    // This is gonna spam the logs
    //trace_log("%s", "[RenderingContext]: Deallocating Rendering Context");

    Py_CLEAR(self->scene);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

float* Py3dRenderingContext_GetCameraPosW(struct Py3dRenderingContext *self)
{
    if (self == NULL) return NULL;

    return self->camera.posW;
}

float* Py3dRenderingContext_GetCameraVPMtx(struct Py3dRenderingContext *self)
{
    if (self == NULL) return NULL;

    return self->camera.vpMtx;
}