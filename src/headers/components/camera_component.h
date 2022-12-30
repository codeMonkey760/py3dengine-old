#ifndef PY3DENGINE_CAMERA_COMPONENT_H
#define PY3DENGINE_CAMERA_COMPONENT_H

#include "components/base_component.h"

#define COMPONENT_TYPE_NAME_CAMERA "camera"

struct CameraComponent {
    struct BaseComponent base;
    float fovxInDegrees;
    unsigned int renderTargetWidth;
    unsigned int renderTargetHeight;
    float nearPlaneDistance;
    float farPlaneDistance;

    float pMtxCache[16];
    bool isMatrixCacheDirty;
};

extern void allocCameraComponent(struct CameraComponent **componentPtr);
extern void deleteCameraComponent(struct CameraComponent **componentPtr);

extern const float *getCameraComponentProjMatrix(struct CameraComponent *component);

extern void setCameraComponentLens(
    struct CameraComponent *component,
    float fovxInDegrees,
    unsigned int renderTargetWidth,
    unsigned int renderTargetHeight,
    float nearPlaneDistance,
    float farPlaneDistance
);

extern void cameraComponentZoom(struct CameraComponent *component, float displacement);
extern void setCameraComponentFov(struct CameraComponent *component, float newFovXInDegrees);

extern void resizeCameraComponentRenderTarget(
    struct CameraComponent *component,
    unsigned int newRenderTargetWidth,
    unsigned int newRenderTargetHeight
);

extern void setCameraComponentNearPlaceDistance(struct CameraComponent *component, float newNearPlaneDistance);
extern void setCameraComponentFarPlaneDistance(struct CameraComponent *component, float newFarPlaneDistance);

#endif
