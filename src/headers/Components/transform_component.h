#ifndef PY3DENGINE_TRANSFORM_COMPONENT_H
#define PY3DENGINE_TRANSFORM_COMPONENT_H

#include "Components/base_component.h"

#define COMPONENT_TYPE_NAME_TRANSFORM "transform"

struct TransformComponent {
    struct BaseComponent base;

    // TODO: these are going to be defined in world space until matrix chaining is implemented
    // after that, these values will be defined relative to the owner's space
    float _position[3];
    float _orientation[4];
    float _scale[3];

    float _wMtxCache[16];
    float _witMtxCache[16];
    bool _matrixCacheDirty;
};

extern void allocTransformComponent(struct TransformComponent **componentPtr);
extern void deleteTransformComponent(struct TransformComponent **componentPtr);

extern void moveTransform(struct TransformComponent *component, float displacement[3]);
extern void setTransformPosition(struct TransformComponent *component, float newPosition[3]);

extern void rotateTransform(struct TransformComponent *component, float displacement[4]);
extern void setTransformOrientation(struct TransformComponent *component, float newOrientation[4]);

extern void stretchTransform(struct TransformComponent *component, float factors[3]);
extern void setTransformScale(struct TransformComponent *component, float newScale[3]);

extern float *getTransformWorldMtx(struct TransformComponent *component);
extern float *getTransformWITMtx(struct TransformComponent *component);

#endif
