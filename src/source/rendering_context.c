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

    (*contextPtr) = context;
    context = NULL;
}

void deleteRenderingContext(struct RenderingContext **contextPtr) {
    if (contextPtr == NULL || (*contextPtr) == NULL) return;

    free( (*contextPtr) );
    (*contextPtr) = NULL;
}

void initRenderingContext(struct RenderingContext *context, struct GameObject *activeCamera) {
    if (context == NULL || activeCamera == NULL) return;

    struct Py3dTransform *transform = getGameObjectTransform(activeCamera);
    if (transform == NULL) {
        critical_log("%s", "[RenderingContext]: Could not query Transform Component from the active camera game object. It's probably malformed.");
        return;
    }


    struct PerspectiveCamera *camera = NULL;
    size_t numComponents = 0;
    numComponents = getGameObjectComponentsLength(activeCamera);
    for (size_t i = 0; i < numComponents; ++i) {
        struct Py3dComponent *curComponent = getGameObjectComponentByIndex(activeCamera, i);
        extractCameraFromComponent((PyObject *) curComponent, &camera);

        if (camera != NULL) break;
    }

    if (camera == NULL) {
        error_log("%s", "[RenderingContext]: Could not query Camera Component from the active camera game object. It's probably malformed.");
        return;
    }

    float pMtx[16] = {0.0f};
    // TODO: when rendering targets are invented get the dimensions from it
    // right now, the engine is the rendering target
    int width = 0, height = 0;
    getRenderingTargetDimensions(&width, &height);
    buildPerspectiveMatrix(pMtx, camera, width, height);

    Mat4Mult(
        context->vpMtx,
        getTransformViewMtx(getGameObjectTransform(activeCamera)),
        pMtx
    );

    Vec3Copy(context->cameraPositionW, transform->position);
}
