#ifndef PY3DENGINE_MODEL_RENDERER_COMPONENT_H
#define PY3DENGINE_MODEL_RENDERER_COMPONENT_H

#include "components/base_component.h"

#define COMPONENT_TYPE_NAME_MODEL_RENDERER "model_renderer"

struct Model;
struct Shader;
struct Material;

struct ModelRendererComponent {
    struct BaseComponent base;
    struct Shader *shader;
    struct Model *model;
    struct Material *material;
};

extern void allocModelRendererComponent(struct ModelRendererComponent **componentPtr);
extern void deleteModelRendererComponent(struct ModelRendererComponent **componentPtr);

extern void setModelRendererComponentShader(struct ModelRendererComponent *component, struct Shader *newShader);
extern void setModelRendererComponentModel(struct ModelRendererComponent *component, struct Model *newModel);
extern void setModelRendererComponentMaterial(struct ModelRendererComponent *component, struct Material *newMaterial);

#endif
