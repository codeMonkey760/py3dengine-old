#ifndef PY3DENGINE_ROTATION_COMPONENT_H
#define PY3DENGINE_ROTATION_COMPONENT_H

#include "Components/base_component.h"

#define COMPONENT_TYPE_NAME_ROTATION "rotation"

struct RotationComponent {
    struct BaseComponent base;
    float _rotSpeed;
    float _rotAxis[3];
};

extern void allocRotationComponent(struct RotationComponent **componentPtr);
extern void deleteRotationComponent(struct RotationComponent **componentPtr);

extern void setRotationComponentSpeed(struct RotationComponent *component, float newSpeed);
extern void setRotationComponentAxis(struct RotationComponent *component, float newAxis[3]);

#endif
