#include "util.h"
#include "Components/model_renderer_component.h"
#include "game_object.h"
#include "Components/transform_component.h"

#define COMPONENT_TYPE_MODEL_RENDERER 2

static bool isComponentValid(struct BaseComponent *component) {
    if (component == NULL) return false;

    return component->_type == COMPONENT_TYPE_MODEL_RENDERER;
}

static void render(struct BaseComponent *component, struct Camera *camera) {
    if (!isComponentValid(component) || camera == NULL) return;

    struct ModelRendererComponent *mrc = (struct ModelRendererComponent *) component;

    if (mrc->shader == NULL || mrc->model == NULL) return;

    struct TransformComponent *transform = getGameObjectTransform(getComponentOwner(component));
    if (transform == NULL) return;

    enableShader(mrc->shader);
    float color[3] = {0.0f};
    Vec3Fill(color, 1.0f);
    setDiffuseColor(mrc->shader, color);

    float cameraPosition[3] = {0.0f};
    getCameraPositionW(camera, cameraPosition);
    setCameraPosition(mrc->shader, cameraPosition);

    setWMtx(mrc->shader, getTransformWorldMtx(transform));
    setWITMtx(mrc->shader, getTransformWITMtx(transform));

    float wvpMtx[16] = {0.0f};
    getVPMtx(camera, wvpMtx);
    Mat4Mult(wvpMtx, getTransformWorldMtx(transform), wvpMtx);
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
    base->render = render;
    base->delete = delete;

    newComponent->shader = NULL;
    newComponent->model = NULL;

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
