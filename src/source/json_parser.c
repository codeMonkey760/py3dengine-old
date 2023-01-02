#include <string.h>
#include <json-c/json.h>

#include "logger.h"
#include "util.h"
#include "config.h"
#include "json_parser.h"
#include "game_object.h"
#include "resource_manager.h"
#include "resources/model.h"
#include "resources/shader.h"
#include "resources/material.h"
#include "components/component_factory.h"
#include "components/base_component.h"
#include "components/transform_component.h"
#include "components/camera_component.h"
#include "components/model_renderer_component.h"
#include "components/rotation_component.h"

#define TYPE_NAME_MAX_SIZE 64

static json_object *fetchProperty(json_object *parent, const char *key, json_type type) {
    if (parent == NULL || key == NULL) return NULL;

    json_object *ret = json_object_object_get(parent, key);
    if (ret == NULL) {
        error_log("[JsonParser]: Could not fetch field named \"%s\"", key);

        return NULL;
    }
    if (!json_object_is_type(ret, type)) {
        error_log(
            "[JsonParser]: Type check failure. Expected \"%s\" to be \"%s\", but got \"%s\" instead",
            key,
            json_type_to_name(type),
            json_type_to_name(json_object_get_type(ret))
        );

        return NULL;
    }

    return ret;
}

static bool parseVec(json_object *json, const char *name, float dst[4], size_t vecSize) {
    json_object *vec = fetchProperty(json, name, json_type_object);
    if (vec == NULL) return false;

    char elements[8] = {'x', 0, 'y', 0, 'z', 0, 'w', 0};
    for (int i = 0; i < vecSize; ++i) {
        char *element_name = elements+(i*2);
        json_object *element = fetchProperty(vec, element_name, json_type_double);
        if (element == NULL) return false;

        dst[i] = (float) json_object_get_double(element);
    }

    return true;
}

bool parseRotationComponent(struct RotationComponent *component, json_object *json) {
    if (component == NULL || json == NULL) return false;

    json_object *json_name = fetchProperty(json, "name", json_type_string);
    if (json_name == NULL) return false;

    float newAxis[3];
    Vec3Fill(newAxis, 0.0f);
    if (!parseVec(json, "axis", newAxis, 3)) return false;

    json_object *json_speed = fetchProperty(json, "speed", json_type_double);
    if (json_speed == NULL) return false;

    setComponentName((struct BaseComponent *) component, json_object_get_string(json_name));
    setRotationComponentAxis(component, newAxis);
    setRotationComponentSpeed(component, (float) json_object_get_double(json_speed));

    return true;
}

bool parseModelRendererComponent(struct ModelRendererComponent *component, json_object *json, struct ResourceManager *manager) {
    if (component == NULL || json == NULL || manager == NULL) return false;

    json_object *json_name = fetchProperty(json, "name", json_type_string);
    if (json_name == NULL) return false;

    json_object *json_model_name = fetchProperty(json, "model", json_type_string);
    if (json_model_name == NULL) return false;

    json_object *json_shader_name = fetchProperty(json, "shader", json_type_string);
    if (json_shader_name == NULL) return false;

    json_object *json_material_name = fetchProperty(json, "material", json_type_string);
    if (json_material_name == NULL) return false;

    setComponentName((struct BaseComponent *) component, json_object_get_string(json_name));

    const char *newModelName = json_object_get_string(json_model_name);
    struct BaseResource *newModel = getResource(manager, newModelName);
    if (newModel != NULL || !isResourceTypeModel(newModel)) {
        setModelRendererComponentModel(component, (struct Model *) newModel);
    } else {
        warning_log("[JsonParser]: Model resource look up failed for \"%s\" while parsing component", newModelName);
    }

    const char *newShaderName = json_object_get_string(json_shader_name);
    struct BaseResource *newShader = getResource(manager, newShaderName);
    if (newShader != NULL || !isResourceTypeShader(newShader)) {
        setModelRendererComponentShader(component, (struct Shader *) newShader);
    } else {
        warning_log("[JsonParser]: Shader resource look up failed for \"%s\" while parsing component", newShaderName);
    }

    const char *newMaterialName = json_object_get_string(json_material_name);
    struct BaseResource *newMaterial = getResource(manager, newMaterialName);
    if (newMaterial != NULL || !isResourceTypeMaterial(newMaterial)) {
        setModelRendererComponentMaterial(component, (struct Material *) newMaterial);
    } else {
        warning_log("[JsonParser]: Material resource look up failed for \"%s\" while parsing component", newMaterialName);
    }

    return true;
}

bool parseCameraComponent(struct CameraComponent *component, json_object *json) {
    if (component == NULL || json == NULL) return false;

    json_object *json_name = fetchProperty(json, "name", json_type_string);
    if (json_name == NULL) return false;

    json_object *json_fovx = fetchProperty(json, "fovx", json_type_double);
    if (json_fovx == NULL) return false;

    json_object *json_near = fetchProperty(json, "near", json_type_double);
    if (json_near == NULL) return false;

    json_object *json_far = fetchProperty(json, "far", json_type_double);
    if (json_far == NULL) return false;

    setComponentName((struct BaseComponent *) component, json_object_get_string(json_name));
    // TODO: get render target dimensions properly, this is wrong
    // maybe render target dimensions shouldn't come from the camera?
    // maybe they should come from the ... render target?
    // remember this when I refactor and expand the rendering pipeline
    setCameraComponentLens(
            component,
            (float) json_object_get_double(json_fovx),
            getConfigScreenWidth(),
            getConfigScreenHeight(),
            (float) json_object_get_double(json_near),
            (float) json_object_get_double(json_far)
    );

    return true;
}

bool parseTransformComponent(json_object *json, struct TransformComponent *component) {
    if (json == NULL || component == NULL) return false;

    float dataBuffer[4];
    memset(dataBuffer, 0, sizeof(float) * 4);

    if (parseVec(json, "position", dataBuffer, 3) == false) return false;
    Vec3Copy(component->_position, dataBuffer);

    if (parseVec(json, "orientation", dataBuffer, 4) == false) return false;
    QuaternionCopy(component->_orientation, dataBuffer);

    if (parseVec(json, "scale", dataBuffer, 3) == false) return false;
    Vec3Copy(component->_scale, dataBuffer);

    return true;
}

bool parseComponentByType(
    struct BaseComponent *component,
    const char *typeName,
    json_object *json,
    struct ResourceManager *resourceManager
) {
    if (component == NULL || typeName == NULL || json == NULL || resourceManager == NULL) return false;

    if (strncmp(COMPONENT_TYPE_NAME_CAMERA, typeName, TYPE_NAME_MAX_SIZE) == 0) {
        return parseCameraComponent((struct CameraComponent *) component, json);
    } else if (strncmp(COMPONENT_TYPE_NAME_MODEL_RENDERER, typeName, TYPE_NAME_MAX_SIZE) == 0) {
        return parseModelRendererComponent((struct ModelRendererComponent *) component, json, resourceManager);
    } else if (strncmp(COMPONENT_TYPE_NAME_ROTATION, typeName, TYPE_NAME_MAX_SIZE) == 0) {
        return parseRotationComponent((struct RotationComponent *) component, json);
    } else {
        error_log("[JsonParser]: Unable to resolve typeName \"%s\"", typeName);
    }

    return false;
}

bool parseGameObject(
        json_object *json,
        struct GameObject *parent,
        struct GameObject **rootPtr,
        struct ResourceManager *resourceManager
) {
    if (json == NULL || rootPtr == NULL || (*rootPtr) != NULL || resourceManager == NULL) return false;

    json_object *json_name = fetchProperty(json, "name", json_type_string);
    if (json_name == NULL) return false;

    json_object *json_transform = fetchProperty(json, "transform", json_type_object);
    if (json_transform == NULL) return false;

    json_object *json_components_array = fetchProperty(json, "components", json_type_array);
    if (json_components_array == NULL) return false;

    json_object *json_children_array = fetchProperty(json, "children", json_type_array);
    if (json_children_array == NULL) return false;

    struct GameObject *newGO = NULL;
    allocGameObject(&newGO);
    if (newGO == NULL) return false;

    setGameObjectName(newGO, json_object_get_string(json_name));
    parseTransformComponent(json_transform, newGO->transform);

    size_t json_components_array_length = json_object_array_length(json_components_array);
    for (size_t i = 0; i < json_components_array_length; ++i) {
        json_object *cur_component_json = json_object_array_get_idx(json_components_array, i);
        if (cur_component_json == NULL || !json_object_is_type(cur_component_json, json_type_object)) {
            error_log(
                    "[JsonParser]: Could not parse component of Game Object with name \"%s\"",
                    getGameObjectName(newGO)
            );

            continue;
        }

        json_object *type_name_json = fetchProperty(cur_component_json, "type", json_type_string);
        if (type_name_json == NULL) continue;
        const char *typeName = json_object_get_string(type_name_json);

        struct BaseComponent *newComponent = NULL;
        componentFactoryCreateComponentFromJson(typeName, &newComponent);
        if (newComponent == NULL) continue;

        if (!parseComponentByType(newComponent, typeName, cur_component_json, resourceManager)) {
            error_log("%s", "[JsonParser]: Component failed to parse. Discarding it.");
            deleteComponent(&newComponent);

            continue;
        }

        attachComponent(newGO, newComponent);
        newComponent = NULL;
    }

    size_t json_children_array_length = json_object_array_length(json_children_array);
    for (size_t i = 0; i < json_children_array_length; ++i) {
        json_object *cur_child_json = json_object_array_get_idx(json_children_array, i);
        if (cur_child_json == NULL || !json_object_is_type(cur_child_json, json_type_object)) {
            error_log(
                    "[JsonParser]: Could not parse child of Game Object with name \"%s\"",
                    getGameObjectName(newGO)
            );

            continue;
        }

        parseGameObject(cur_child_json, newGO, rootPtr, resourceManager);
    }

    if (parent != NULL) {
        attachChild(parent, newGO);
    } else {
        (*rootPtr) = newGO;
    }
    newGO = NULL;
}