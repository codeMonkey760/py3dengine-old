#include <stdlib.h>

#include "util.h"
#include "Components/rotation_component.h"
#include "game_object.h"
#include "Components/transform_component.h"

#define COMPONENT_TYPE_ROTATION 1

static bool isComponentValid(struct BaseComponent *component) {
    if (component == NULL) return false;

    return component->_type == COMPONENT_TYPE_ROTATION;
}

static void update(struct BaseComponent *component, float dt) {
    if (!isComponentValid(component)) return;

    struct RotationComponent *rotCom = (struct RotationComponent *) component;

    float deltaRot[4];
    QuaternionIdentity(deltaRot);
    QuaternionFromAxisAngle(
        rotCom->_rotAxis[0],
        rotCom->_rotAxis[1],
        rotCom->_rotAxis[2],
        dt * rotCom->_rotSpeed,
        deltaRot
    );

    struct TransformComponent *transform = getGameObjectTransform(getComponentOwner(component));
    if (transform == NULL) return;
    rotateTransform(transform, deltaRot);
}

static void delete(struct BaseComponent **componentPtr) {
    if (componentPtr == NULL) return;

    if (!isComponentValid((*componentPtr))) return;

    deleteRotationComponent((struct RotationComponent **) componentPtr);
}

extern void allocRotationComponent(struct RotationComponent **componentPtr){
    if (componentPtr == NULL || (*componentPtr) != NULL) return;

    struct RotationComponent *newComponent = calloc(1, sizeof(struct RotationComponent));
    if (newComponent == NULL) return;

    struct BaseComponent *base = (struct BaseComponent *) newComponent;
    initializeBaseComponent(base);
    base->_type = COMPONENT_TYPE_ROTATION;
    base->update = update;
    base->delete = delete;

    newComponent->_rotSpeed = 0.0f;
    Vec3Identity(newComponent->_rotAxis);
    newComponent->_rotAxis[2] = 1.0f;

    (*componentPtr) = newComponent;
    newComponent = NULL;
    base = NULL;
}

extern void deleteRotationComponent(struct RotationComponent **componentPtr) {
    if (componentPtr == NULL || (*componentPtr) == NULL) return;

    finalizeBaseComponent((struct BaseComponent *) (*componentPtr));

    free( (*componentPtr) );
    (*componentPtr) = NULL;
}

extern void setRotationComponentSpeed(struct RotationComponent *component, float newSpeed) {
    if (component == NULL) return;

    component->_rotSpeed = newSpeed;
}

extern void setRotationComponentAxis(struct RotationComponent *component, float newAxis[3]) {
    if (component == NULL || newAxis == NULL) return;

    Vec3Copy(component->_rotAxis, newAxis);
}
