#ifndef PY3DENGINE_MODEL_H
#define PY3DENGINE_MODEL_H

#include <stdlib.h>

struct Model {
    unsigned int _vao;
    unsigned int _vbo;
    size_t _sizeInVertices;

    struct String *_name;
};

extern void allocModel(struct Model **modelPtr);
extern void deleteModel(struct Model **modelPtr);

extern void setPNTBuffer(struct Model *model, const float *buffer, size_t bufferSizeInElements);

extern void bindModel(struct Model *model);
extern void unbindModel(struct Model *model);
extern void renderModel(struct Model *model);

extern struct String *getModelName(struct Model *model);
extern void setModelName(struct Model *model, const char *newName);

#endif