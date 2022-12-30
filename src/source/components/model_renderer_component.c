#include "util.h"
#include "custom_string.h"
#include "model.h"
#include "shader.h"
#include "material.h"
#include "components/model_renderer_component.h"
#include "game_object.h"
#include "components/transform_component.h"
#include "rendering_context.h"

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
