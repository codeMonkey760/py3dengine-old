#ifndef PY3DENGINE_RESOURCE_MANAGER_H
#define PY3DENGINE_RESOURCE_MANAGER_H

#include "resources/shader.h"
#include "resources/model.h"
#include "resources/material.h"

struct ResourceManager {
    struct ListNode *_root;
};

extern void allocResourceManager(struct ResourceManager **resourceManagerPtr);
extern void deleteResourceManager(struct ResourceManager **resourceManagerPtr);

extern void storeShader(struct ResourceManager *manager, struct Shader *shader);
extern void storeModel(struct ResourceManager *manager, struct Model *model);
extern void storeMaterial(struct ResourceManager *manager, struct Material *material);

extern struct Shader * getShaderResource(struct ResourceManager *manager, const char *name);
extern struct Model * getModelResource(struct ResourceManager *manager, const char *name);
extern struct Material * getMaterialResource(struct ResourceManager *manager, const char *name);

#endif
