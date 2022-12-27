#include "Components/model_renderer_component.h"

#define COMPONENT_TYPE_MODEL_RENDERER 2

static bool isComponentValid(void *component) {
    if (component == NULL) return false;

    struct BaseComponent *base = (struct BaseComponent *) component;

    return base->_type == COMPONENT_TYPE_MODEL_RENDERER;
}

static void render(void *component, struct Camera *camera) {
    if (!isComponentValid(component) || camera == NULL) return;

    struct ModelRendererComponent *mrc = (struct ModelRendererComponent *) component;

    // TODO: copy the stuff in quad::render
}

static void delete(void **componentPtr) {
    if (componentPtr == NULL) return;

    if (!isComponentValid( (*componentPtr) )) return;

    deleteModelRendererComponent((struct ModelRendererComponent **) componentPtr);
}

void allocModelRendererComponent(struct ModelRendererComponent **componentPtr) {
    if (componentPtr == NULL || (*componentPtr) != NULL) return;

    struct ModelRendererComponent *newComponent = calloc(1, sizeof(struct ModelRendererComponent));
    if (newComponent == NULL) return;

    struct BaseComponent *base = (struct BaseComponent *) newComponent;
    base->_type = COMPONENT_TYPE_MODEL_RENDERER;
    base->update = NULL;
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
