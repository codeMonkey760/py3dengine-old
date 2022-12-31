#include <stdlib.h>

#include "util.h"
#include "logger.h"
#include "custom_string.h"
#include "components/rotation_component.h"
#include "game_object.h"
#include "components/transform_component.h"

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

static bool parse(struct BaseComponent *component, json_object *json, struct ResourceManager *resource) {
    if (!isComponentValid(component) || json == NULL || resource == NULL) return false;

    json_object *json_name = json_object_object_get(json, "name");
    if (json_name == NULL || !json_object_is_type(json_name, json_type_string)) {
        error_log("%s", "[RotationComponent]: Component must have a string property called \"name\"");
        return false;
    }

    json_object *json_axis = json_object_object_get(json, "axis");
    if (json_axis == NULL || !json_object_is_type(json_axis, json_type_object)) {
        error_log("%s", "[RotationComponent]: Component must have an object property called \"axis\"");
        return false;
    }

    json_object *json_axis_x = json_object_object_get(json_axis, "x");
    if (json_axis_x == NULL || !json_object_is_type(json_axis_x, json_type_double)) {
        error_log("%s", "RotationComponent]: Component must have a double property called \"x\" assigned to \"axis\"");
        return false;
    }

    json_object *json_axis_y = json_object_object_get(json_axis, "y");
    if (json_axis_y == NULL || !json_object_is_type(json_axis_x, json_type_double)) {
        error_log("%s", "RotationComponent]: Component must have a double property called \"y\" assigned to \"axis\"");
        return false;
    }

    json_object *json_axis_z = json_object_object_get(json_axis, "z");
    if (json_axis_z == NULL || !json_object_is_type(json_axis_z, json_type_double)) {
        error_log("%s", "RotationComponent]: Component must have a double property called \"z\" assigned to \"axis\"");
        return false;
    }

    json_object *json_speed = json_object_object_get(json, "speed");
    if (json_speed == NULL || !json_object_is_type(json_speed, json_type_double)) {
        error_log("%s", "[RotationComponent]: Component must have a double property called \"speed\"");
        return false;
    }

    setComponentName(component, json_object_get_string(json_name));

    struct RotationComponent *rc = (struct RotationComponent *) component;

    float newAxis[3] = {0.0f};
    newAxis[0] = (float) json_object_get_double(json_axis_x);
    newAxis[1] = (float) json_object_get_double(json_axis_y);
    newAxis[2] = (float) json_object_get_double(json_axis_z);

    setRotationComponentAxis(rc, newAxis);
    setRotationComponentSpeed(rc, (float) json_object_get_double(json_speed));

    return true;
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
    allocString(&base->_typeName, COMPONENT_TYPE_NAME_ROTATION);
    base->update = update;
    base->parse = parse;
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
