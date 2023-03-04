#ifndef PY3DENGINE_RESOURCES_SHADER_H
#define PY3DENGINE_RESOURCES_SHADER_H

#include "resources/base_resource.h"

#define RESOURCE_TYPE_NAME_SHADER "Shader"

struct Texture;
struct UniformListNode;

struct Shader {
    struct BaseResource _base;

    unsigned int _vertexShader;
    unsigned int _fragShader;
    unsigned int _program;

    struct UniformListNode *uniformList;
};

extern bool isResourceTypeShader(struct BaseResource *resource);

extern void allocShader(struct Shader **shaderPtr);
extern void deleteShader(struct Shader **shaderPtr);

extern void initShaderFromFiles(struct Shader *shader, const char *vertexShaderFileName, const char *fragShaderFileName);
extern void initShader(struct Shader *shader, const char *vertexShaderSource, const char *fragShaderSource);
extern void enableShader(struct Shader *shader);
extern void disableShader(struct Shader *shader);

extern bool setShaderFloatArrayUniform(struct Shader *shader, const char *name, const float *src, size_t numElements);
extern bool setShaderMatrixUniform(struct Shader *shader, const char *name, const float *src, size_t dimensions);
extern bool setShaderTextureUniform(struct Shader *shader, const char *name, struct Texture *texture);

#endif
