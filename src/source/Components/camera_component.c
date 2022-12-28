#include <stdlib.h>

#include "util.h"
#include "custom_string.h"
#include "Components/camera_component.h"

#define COMPONENT_TYPE_CAMERA 4

static bool isComponentValid(struct BaseComponent *component) {
    if (component == NULL) return false;

    return component->_type == COMPONENT_TYPE_CAMERA;
}

static void delete(struct BaseComponent **componentPtr) {
    if (componentPtr == NULL) return;

    if (!isComponentValid( (*componentPtr) )) return;

    deleteCameraComponent((struct CameraComponent **) componentPtr);
}

void allocCameraComponent(struct CameraComponent **componentPtr) {
    if (componentPtr == NULL || (*componentPtr) != NULL) return;

    struct CameraComponent *newComponent = calloc(1, sizeof(struct CameraComponent));
    if (newComponent == NULL) return;

    struct BaseComponent *base = (struct BaseComponent *) newComponent;
    initializeBaseComponent(base);
    base->_type = COMPONENT_TYPE_CAMERA;
    allocString(&base->_typeName, COMPONENT_TYPE_NAME_CAMERA);
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

// TODO: implement CameraComponent::getProjMatrix

void setLens(
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

void zoom(struct CameraComponent *component, float displacement) {
    if (component == NULL) return;

    component->fovxInDegrees += displacement;
    component->isMatrixCacheDirty = true;
}

void setFov(struct CameraComponent *component, float newFovXInDegrees) {
    if (component == NULL) return;

    component->fovxInDegrees = newFovXInDegrees;
    component->isMatrixCacheDirty = true;
}

void resizeRenderTarget(
    struct CameraComponent *component,
    unsigned int newRenderTargetWidth,
    unsigned int newRenderTargetHeight
) {
    if (component == NULL) return;

    component->renderTargetWidth = newRenderTargetWidth;
    component->renderTargetHeight = newRenderTargetHeight;
    component->isMatrixCacheDirty = true;
}

void setNearPlaceDistance(struct CameraComponent *component, float newNearPlaneDistance) {
    if (component == NULL) return;

    component->nearPlaneDistance = newNearPlaneDistance;
    component->isMatrixCacheDirty = true;
}

void setFarPlaneDistance(struct CameraComponent *component, float newFarPlaneDistance) {
    if (component == NULL) return;

    component->farPlaneDistance = newFarPlaneDistance;
    component->isMatrixCacheDirty = true;
}
