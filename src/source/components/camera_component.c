#include <stdlib.h>

#include "config.h"
#include "util.h"
#include "logger.h"
#include "custom_string.h"
#include "components/camera_component.h"

#define COMPONENT_TYPE_CAMERA 4

static bool isComponentValid(struct BaseComponent *component) {
    if (component == NULL) return false;

    return component->_type == COMPONENT_TYPE_CAMERA;
}

static void resize(struct BaseComponent *component, int newWidth, int newHeight) {
    if (!isComponentValid(component)) return;

    resizeCameraComponentRenderTarget(
        (struct CameraComponent *) component,
        newWidth,
        newHeight
    );
}

static bool parse(struct BaseComponent *component, json_object *json, struct ResourceManager *resourceManager) {
    if (!isComponentValid(component) || json == NULL || resourceManager == NULL) return false;

    json_object *json_name = json_object_object_get(json, "name");
    if (json_name == NULL || !json_object_is_type(json_name, json_type_string)) {
        error_log("%s", "[CameraComponent]: Component must have a string property called \"name\"");
        return false;
    }

    json_object *json_fovx = json_object_object_get(json, "fovx");
    if (json_fovx == NULL || !json_object_is_type(json_fovx, json_type_double)) {
        error_log("%s", "[CameraComponent]: Component must have a double property called \"fovx\"");
        return false;
    }

    json_object *json_near = json_object_object_get(json, "near");
    if (json_near == NULL || !json_object_is_type(json_near, json_type_double)) {
        error_log("%s", "[CameraComponent]: Component must have a double property called \"near\"");
        return false;
    }

    json_object *json_far = json_object_object_get(json, "far");
    if (json_far == NULL || !json_object_is_type(json_far, json_type_double)) {
        error_log("%s", "[CameraComponent]: Component must have a double property called \"far\"");
        return false;
    }

    setComponentName(component, json_object_get_string(json_name));

    // TODO: get render target dimensions properly, this is wrong
    setCameraComponentLens(
        (struct CameraComponent *) component,
        (float) json_object_get_double(json_fovx),
        getConfigScreenWidth(),
        getConfigScreenHeight(),
        (float) json_object_get_double(json_near),
        (float) json_object_get_double(json_far)
    );

    return true;
}

static void delete(struct BaseComponent **componentPtr) {
    if (componentPtr == NULL) return;

    if (!isComponentValid( (*componentPtr) )) return;

    deleteCameraComponent((struct CameraComponent **) componentPtr);
}

static void refreshProjMatrixCache(struct CameraComponent *component) {
    if (component == NULL || component->isMatrixCacheDirty == false) return;

    float aspectRatio = ((float) component->renderTargetWidth) / ((float) component->renderTargetHeight);
    float aspectRatioInv = ((float) component->renderTargetHeight) / ((float) component->renderTargetWidth);
    float fov_y_radians = DEG_TO_RAD(aspectRatioInv * component->fovxInDegrees);
    float w = 1.0f / (aspectRatio * tanf(fov_y_radians / 2.0f));
    float h = 1.0f / (tanf(fov_y_radians / 2.0f));
    float far_z = component->farPlaneDistance;
    float near_z = component->nearPlaneDistance;

    Mat4Identity(component->pMtxCache);
    component->pMtxCache[0] = w;
    component->pMtxCache[5] = h;
    component->pMtxCache[10] = far_z / (far_z - near_z);
    component->pMtxCache[11] = 1.0f;
    component->pMtxCache[14] = (-1.0f * near_z * far_z) / (far_z - near_z);

    component->isMatrixCacheDirty = false;
}

void allocCameraComponent(struct CameraComponent **componentPtr) {
    if (componentPtr == NULL || (*componentPtr) != NULL) return;

    struct CameraComponent *newComponent = calloc(1, sizeof(struct CameraComponent));
    if (newComponent == NULL) return;

    struct BaseComponent *base = (struct BaseComponent *) newComponent;
    initializeBaseComponent(base);
    base->_type = COMPONENT_TYPE_CAMERA;
    allocString(&base->_typeName, COMPONENT_TYPE_NAME_CAMERA);
    base->resize = resize;
    base->parse = parse;
    base->delete = delete;
    base = NULL;

    newComponent->fovxInDegrees = 0.0f;
    newComponent->renderTargetWidth = 0;
    newComponent->renderTargetHeight = 0;
    newComponent->nearPlaneDistance = 0.0f;
    newComponent->farPlaneDistance = 0.0f;

    Mat4Identity(newComponent->pMtxCache);
    newComponent->isMatrixCacheDirty = true;

    (*componentPtr) = newComponent;
    newComponent = NULL;
}

void deleteCameraComponent(struct CameraComponent **componentPtr) {
    if (componentPtr == NULL || (*componentPtr) == NULL) return;

    finalizeBaseComponent((struct BaseComponent *) (*componentPtr));

    free( (*componentPtr) );
    (*componentPtr) = NULL;
}

const float *getCameraComponentProjMatrix(struct CameraComponent *component) {
    if (component == NULL) return NULL;

    refreshProjMatrixCache(component);

    return component->pMtxCache;
}

void setCameraComponentLens(
    struct CameraComponent *component,
    float fovxInDegrees,
    unsigned int renderTargetWidth,
    unsigned int renderTargetHeight,
    float nearPlaneDistance,
    float farPlaneDistance
) {
    if (component == NULL) return;

    component->fovxInDegrees = fovxInDegrees;
    component->renderTargetWidth = renderTargetWidth;
    component->renderTargetHeight = renderTargetHeight;
    component->nearPlaneDistance = nearPlaneDistance;
    component->farPlaneDistance = farPlaneDistance;
}

void cameraComponentZoom(struct CameraComponent *component, float displacement) {
    if (component == NULL) return;

    component->fovxInDegrees += displacement;
    component->isMatrixCacheDirty = true;
}

void setCameraComponentFov(struct CameraComponent *component, float newFovXInDegrees) {
    if (component == NULL) return;

    component->fovxInDegrees = newFovXInDegrees;
    component->isMatrixCacheDirty = true;
}

void resizeCameraComponentRenderTarget(
    struct CameraComponent *component,
    unsigned int newRenderTargetWidth,
    unsigned int newRenderTargetHeight
) {
    if (component == NULL) return;

    component->renderTargetWidth = newRenderTargetWidth;
    component->renderTargetHeight = newRenderTargetHeight;
    component->isMatrixCacheDirty = true;
}

void setCameraComponentNearPlaceDistance(struct CameraComponent *component, float newNearPlaneDistance) {
    if (component == NULL) return;

    component->nearPlaneDistance = newNearPlaneDistance;
    component->isMatrixCacheDirty = true;
}

void setCameraComponentFarPlaneDistance(struct CameraComponent *component, float newFarPlaneDistance) {
    if (component == NULL) return;

    component->farPlaneDistance = newFarPlaneDistance;
    component->isMatrixCacheDirty = true;
}
