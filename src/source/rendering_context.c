#include <stdlib.h>

#include "logger.h"
#include "util.h"
#include "rendering_context.h"
#include "game_object.h"
#include "config.h"
#include "python/py3dtransform.h"
#include "python/python_util.h"
#include "engine.h"

struct PerspectiveCamera {
    float fovXInDegrees;
    float nearPlaneDistance;
    float farPlaceDistance;
};

static PyObject *Py3dRenderingContext_Ctor = NULL;

static PyMethodDef Py3dRenderingContext_Methods[] = {
    {NULL}
};

static void Py3dRenderingContext_Dealloc(struct RenderingContext *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dRenderingContext_Init(struct Py3dRenderingContext *self, PyObject *args, PyObject *kwds) {
    self->renderingContext = NULL;

    return 0;
}

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

bool PyInit_Py3dRenderingContext(PyObject *module) {
    if (PyType_Ready(&Py3dRenderingContext_Type) < 0) return false;

    if (PyModule_AddObject(module, "RenderingContext", (PyObject *) &Py3dRenderingContext_Type) < 0) return false;

    Py_INCREF(&Py3dRenderingContext_Type);

    return true;
}

bool Py3dRenderingContext_FindCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "RenderingContext") == 0) {
        critical_log("%s", "[Python]: Py3dRenderingContext has not been initialized properly");

        return false;
    }

    Py3dRenderingContext_Ctor = PyObject_GetAttrString(module, "RenderingContext");
    Py_INCREF(Py3dRenderingContext_Ctor);

    return true;
}

void Py3dRenderingContext_FinalizeCtor() {
    Py_CLEAR(Py3dRenderingContext_Ctor);
}

struct Py3dRenderingContext *Py3dRenderingContext_New() {
    if (Py3dRenderingContext_Ctor == NULL) {
        critical_log("%s", "[Python]: Py3dRenderingContext has not been initialized properly");

        return NULL;
    }

    PyObject *py3dRenderingContext = PyObject_CallNoArgs(Py3dRenderingContext_Ctor);
    if (py3dRenderingContext == NULL) {
        critical_log("%s", "[Python]: Failed to allocate RenderingContext in python interpreter");
        handleException();

        return NULL;
    }

    return (struct Py3dRenderingContext *) py3dRenderingContext;
}

static void allocPerspectiveCamera(struct PerspectiveCamera **cameraPtr) {
    if (cameraPtr == NULL || (*cameraPtr) != NULL) return;

    struct PerspectiveCamera *newCamera = calloc(1, sizeof(struct PerspectiveCamera));
    if (newCamera == NULL) return;

    newCamera->nearPlaneDistance = 1.0f;
    newCamera->farPlaceDistance = 10.0f;
    newCamera->fovXInDegrees = 90.0f;

    (*cameraPtr) = newCamera;
    newCamera = NULL;
}

static void deletePerspectiveCamera(struct PerspectiveCamera **cameraPtr) {
    if (cameraPtr == NULL || (*cameraPtr) == NULL) return;

    free( (*cameraPtr) );
    (*cameraPtr) = NULL;
}

static bool extractFloatFromComponent(PyObject *component, const char *attrName, float *dst) {
    if (component == NULL || attrName == NULL || dst == NULL) return false;

    if (PyObject_HasAttrString(component, attrName) == 0) {
        return false;
    }
    PyObject *attrRawObj = PyObject_GetAttrString(component, attrName);
    if (attrRawObj == NULL) {
        critical_log("[RenderingContext]: Could not extract \"%s\" from component", attrName);
        handleException();
        return false;
    }

    PyObject *attrFltObj = PyNumber_Float(attrRawObj);
    if (attrFltObj == NULL) {
        error_log("[RenderingContext]: Could not convert \"%s\" to float", attrName);
        handleException();
        return false;
    }

    (*dst) = (float) PyFloat_AsDouble(attrFltObj);
    Py_CLEAR(attrFltObj);
    Py_CLEAR(attrRawObj);

    return true;
}

static void extractCameraFromComponent(PyObject *component, struct PerspectiveCamera **cameraPtr) {
    if (component == NULL || cameraPtr == NULL || (*cameraPtr) != NULL) return;

    float fov_x_in_degrees = 0.0f;
    if (!extractFloatFromComponent(component, "fov_x_in_degrees", &fov_x_in_degrees)) return;

    float nearPlaceDistance = 0.0f;
    if (!extractFloatFromComponent(component, "near_z", &nearPlaceDistance)) return;
    
    float farPlaneDistance = 0.0f;
    if (!extractFloatFromComponent(component, "far_z", &farPlaneDistance)) return;

    struct PerspectiveCamera *newCamera = NULL;
    allocPerspectiveCamera(&newCamera);
    if (newCamera == NULL) return;

    newCamera->fovXInDegrees = fov_x_in_degrees;
    newCamera->nearPlaneDistance = nearPlaceDistance;
    newCamera->farPlaceDistance = farPlaneDistance;

    (*cameraPtr) = newCamera;
    newCamera = NULL;
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

void allocRenderingContext(struct RenderingContext **contextPtr) {
    if (contextPtr == NULL || (*contextPtr) != NULL) return;

    struct RenderingContext *context = calloc(1, sizeof(struct RenderingContext));
    if (context == NULL) return;

    Mat4Identity(context->vpMtx);
    Vec3Identity(context->cameraPositionW);
    context->py3dRenderingContext = NULL;
    context->py3dRenderingContext = Py3dRenderingContext_New();
    context->py3dRenderingContext->renderingContext = context;

    (*contextPtr) = context;
    context = NULL;
}

void deleteRenderingContext(struct RenderingContext **contextPtr) {
    if (contextPtr == NULL || (*contextPtr) == NULL) return;

    struct RenderingContext *context = (*contextPtr);
    Py_CLEAR(context->py3dRenderingContext);

    free( (*contextPtr) );
    (*contextPtr) = NULL;
}

void initRenderingContext(struct RenderingContext *context, struct Py3dGameObject *activeCamera) {
    if (context == NULL || activeCamera == NULL) return;

    if (!Py3dGameObject_Check((PyObject *) activeCamera)) {
        critical_log("%s", "[RenderingContext]: Active camera must be a Game Object");
        return;
    }
    struct Py3dGameObject *activeCameraGO = (struct Py3dGameObject *) activeCamera;

    PyObject *getTransformRet = Py3dGameObject_GetTransform(activeCameraGO, NULL);
    if (getTransformRet == NULL || !Py3dTransform_Check(getTransformRet)) {
        critical_log("%s", "[RenderingContext]: Could not query Transform Component from the active camera game object");
        handleException();
        Py_CLEAR(getTransformRet);
        return;
    }
    struct Py3dTransform *transform = (struct Py3dTransform *) getTransformRet;

    struct PerspectiveCamera *camera = NULL;
    Py_ssize_t componentCount = Py3dGameObject_GetComponentCountInt(activeCameraGO);
    for (Py_ssize_t i = 0; i < componentCount; ++i) {
        PyObject *getComponentRet = Py3dGameObject_GetComponentByIndexInt(activeCameraGO, i);
        if (getComponentRet == NULL || !Py3dComponent_Check(getComponentRet)) {
            Py_CLEAR(getComponentRet);
            critical_log("%s", "[RenderingContext]: Encountered malformed component while parsing for camera");
            handleException();
            continue;
        }
        extractCameraFromComponent(getComponentRet, &camera);
        Py_CLEAR(getComponentRet);

        if (camera != NULL) break;
    }

    if (camera == NULL) {
        error_log("%s", "[RenderingContext]: Could not query Camera Component from the active camera game object. It's probably malformed.");
        Py_CLEAR(transform);
        return;
    }

    float pMtx[16] = {0.0f};
    // TODO: when rendering targets are invented get the dimensions from it
    // right now, the engine is the rendering target
    int width = 0, height = 0;
    getRenderingTargetDimensions(&width, &height);
    buildPerspectiveMatrix(pMtx, camera, width, height);
    deletePerspectiveCamera(&camera);

    Mat4Mult(
        context->vpMtx,
        getTransformViewMtx(transform),
        pMtx
    );

    Vec3Copy(context->cameraPositionW, transform->position);
    Py_CLEAR(transform);
}
