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

extern void setLens(
    struct CameraComponent *component,
    float fovxInDegrees,
    unsigned int renderTargetWidth,
    unsigned int renderTargetHeight,
    float nearPlaneDistance,
    float farPlaneDistance
);

#endif
