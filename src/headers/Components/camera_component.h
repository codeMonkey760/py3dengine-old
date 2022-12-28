#ifndef PY3DENGINE_CAMERA_COMPONENT_H
#define PY3DENGINE_CAMERA_COMPONENT_H

#include "Components/base_component.h"

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

extern const float *getProjMatrix(struct CameraComponent *component);

extern void setLens(
    struct CameraComponent *component,
    float fovxInDegrees,
    unsigned int renderTargetWidth,
    unsigned int renderTargetHeight,
    float nearPlaneDistance,
    float farPlaneDistance
);

extern void zoom(struct CameraComponent *component, float displacement);
extern void setFov(struct CameraComponent *component, float newFovXInDegrees);

extern void resizeRenderTarget(
    struct CameraComponent *component,
    unsigned int newRenderTargetWidth,
    unsigned int newRenderTargetHeight
);

extern void setNearPlaceDistance(struct CameraComponent *component, float newNearPlaneDistance);
extern void setFarPlaneDistance(struct CameraComponent *component, float newFarPlaneDistance);

#endif
