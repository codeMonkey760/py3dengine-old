#ifndef PY3DENGINE_MODEL_H
#define PY3DENGINE_MODEL_H

#include <stdlib.h>

#include "resources/base_resource.h"

#define RESOURCE_TYPE_NAME_MODEL "Model"

struct Model {
    struct BaseResource _base;

    unsigned int _vao;
    unsigned int _vbo;
    size_t _sizeInVertices;
};

extern bool isResourceTypeModel(struct BaseResource *resource);
extern void allocModel(struct Model **modelPtr);
extern void deleteModel(struct Model **modelPtr);

extern void setPNTBuffer(struct Model *model, const float *buffer, size_t bufferSizeInElements);

extern void bindModel(struct Model *model);
extern void unbindModel(struct Model *model);
extern void renderModel(struct Model *model);

#endif
