#include "util.h"
#include "logger.h"
#include "custom_string.h"
#include "resources/model.h"
#include "resources/shader.h"
#include "resources/material.h"
#include "components/model_renderer_component.h"
#include "game_object.h"
#include "components/transform_component.h"
#include "rendering_context.h"
#include "resource_manager.h"

#define COMPONENT_TYPE_MODEL_RENDERER 2

static bool isComponentValid(struct BaseComponent *component) {
    if (component == NULL) return false;

    return component->_type == COMPONENT_TYPE_MODEL_RENDERER;
}

static void render(struct BaseComponent *component, struct RenderingContext *renderingContext) {
    if (!isComponentValid(component) || renderingContext == NULL) return;

    struct ModelRendererComponent *mrc = (struct ModelRendererComponent *) component;

    if (mrc->shader == NULL || mrc->model == NULL || mrc->material == NULL) return;

    struct TransformComponent *transform = getGameObjectTransform(getComponentOwner(component));
    if (transform == NULL) return;

    enableShader(mrc->shader);
    float color[3] = {0.0f};
    Vec3Fill(color, 1.0f);
    setDiffuseColor(mrc->shader, getMaterialDiffuseColor(mrc->material));

    setCameraPosition(mrc->shader, renderingContext->cameraPositionW);

    setWMtx(mrc->shader, getTransformWorldMtx(transform));
    setWITMtx(mrc->shader, getTransformWITMtx(transform));

    float wvpMtx[16] = {0.0f};
    Mat4Identity(wvpMtx);
    Mat4Mult(wvpMtx, getTransformWorldMtx(transform), renderingContext->vpMtx);
    setWVPMtx(mrc->shader, wvpMtx);

    bindModel(mrc->model);
    renderModel(mrc->model);
    unbindModel(mrc->model);

    disableShader(mrc->shader);
}

static bool parse(struct BaseComponent *component, json_object *json, struct ResourceManager *manager) {
    if (!isComponentValid(component) || json == NULL || manager == NULL) return false;

    json_object *json_name = json_object_object_get(json, "name");
    if (json_name == NULL || !json_object_is_type(json_name, json_type_string)) {
        error_log("%s", "[ModelRendererComponent]: Component must have a string property called \"name\"");
        return false;
    }

    json_object *json_model_name = json_object_object_get(json, "model");
    if (json_model_name == NULL || !json_object_is_type(json_model_name, json_type_string)) {
        error_log("%s", "[ModelRendererComponent]: Component must have a string property called \"model\"");
        return false;
    }

    json_object *json_shader_name = json_object_object_get(json, "shader");
    if (json_shader_name == NULL || !json_object_is_type(json_shader_name, json_type_string)) {
        error_log("%s", "[ModelRendererComponent]: Component must have a string property called \"shader\"");
        return false;
    }

    json_object *json_material_name = json_object_object_get(json, "material");
    if (json_material_name == NULL || !json_object_is_type(json_material_name, json_type_string)) {
        error_log("%s", "[ModelRendererComponent]: Component must have a string property called \"material\"");
        return false;
    }

    setComponentName(component, json_object_get_string(json_name));

    struct ModelRendererComponent *mrc = (struct ModelRendererComponent *) component;

    const char *newModelName = json_object_get_string(json_model_name);
    struct Model *newModel = getModelResource(manager, newModelName);
    if (newModel != NULL) {
        setModelRendererComponentModel(mrc, newModel);
    } else {
        warning_log("[ModelRendererComponent]: Model resource look up failed for \"%s\" while parsing component", newModelName);
    }

    const char *newShaderName = json_object_get_string(json_shader_name);
    struct Shader *newShader = getShaderResource(manager, newShaderName);
    if (newShader != NULL) {
        setModelRendererComponentShader(mrc, newShader);
    } else {
        warning_log("[ModelRendererComponent]: Shader resource look up failed for \"%s\" while parsing component", newShaderName);
    }

    const char *newMaterialName = json_object_get_string(json_material_name);
    struct Material *newMaterial = getMaterialResource(manager, newMaterialName);
    if (newMaterial != NULL) {
        setModelRendererComponentMaterial(mrc, newMaterial);
    } else {
        warning_log("[ModelRendererComponent]: Material resource look up failed for \"%s\" while parsing component", newMaterialName);
    }

    return true;
}

static void delete(struct BaseComponent **componentPtr) {
    if (componentPtr == NULL) return;

    if (!isComponentValid( (*componentPtr) )) return;

    deleteModelRendererComponent((struct ModelRendererComponent **) componentPtr);
}

void allocModelRendererComponent(struct ModelRendererComponent **componentPtr) {
    if (componentPtr == NULL || (*componentPtr) != NULL) return;

    struct ModelRendererComponent *newComponent = calloc(1, sizeof(struct ModelRendererComponent));
    if (newComponent == NULL) return;

    struct BaseComponent *base = (struct BaseComponent *) newComponent;
    initializeBaseComponent(base);
    base->_type = COMPONENT_TYPE_MODEL_RENDERER;
    allocString(&base->_typeName, COMPONENT_TYPE_NAME_MODEL_RENDERER);
    base->render = render;
    base->parse = parse;
    base->delete = delete;

    newComponent->shader = NULL;
    newComponent->model = NULL;
    newComponent->material = NULL;

    (*componentPtr) = newComponent;
    newComponent = NULL;
    base = NULL;
}

void deleteModelRendererComponent(struct ModelRendererComponent **componentPtr) {
    if (componentPtr == NULL || (*componentPtr) == NULL) return;

    finalizeBaseComponent((struct BaseComponent *) (*componentPtr));

    free( (*componentPtr) );
    (*componentPtr) = NULL;
}

void setModelRendererComponentShader(struct ModelRendererComponent *component, struct Shader *newShader) {
    if (component == NULL) return;

    component->shader = newShader;
}

void setModelRendererComponentModel(struct ModelRendererComponent *component, struct Model *newModel) {
    if (component == NULL) return;

    component->model = newModel;
}

void setModelRendererComponentMaterial(struct ModelRendererComponent *component, struct Material *newMaterial) {
    if (component == NULL) return;

    component->material = newMaterial;
}
