#ifndef PY3DENGINE_MODEL_RENDERER_COMPONENT_H
#define PY3DENGINE_MODEL_RENDERER_COMPONENT_H

#include "Components/base_component.h"
#include "shader.h"
#include "model.h"

struct ModelRendererComponent {
    struct BaseComponent base;
    struct Shader *shader;
    struct Model *model;
};

extern void allocModelRendererComponent(struct ModelRendererComponent **componentPtr);
extern void deleteModelRendererComponent(struct ModelRendererComponent **componentPtr);

extern void setModelRendererComponentShader(struct ModelRendererComponent *component, struct Shader *newShader);
extern void setModelRendererComponentModel(struct ModelRendererComponent *component, struct Model *newModel);

#endif
