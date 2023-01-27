#ifndef PY3DENGINE_MODEL_H
#define PY3DENGINE_MODEL_H

#include <stdlib.h>

#include "resources/base_resource.h"

#define RESOURCE_TYPE_NAME_MODEL "Model"

struct VertexPNT;

struct Model {
    struct BaseResource _base;

    unsigned int _vao;
    unsigned int _vbo;
    size_t _sizeInVertices;
};

extern bool isResourceTypeModel(struct BaseResource *resource);
extern void allocModel(struct Model **modelPtr);
extern void deleteModel(struct Model **modelPtr);

extern void setModelPNTBuffer(struct Model *model, struct VertexPNT *buffer, size_t bufferSizeInVertices);

extern void bindModel(struct Model *model);
extern void unbindModel(struct Model *model);
extern void renderModel(struct Model *model);

#endif
