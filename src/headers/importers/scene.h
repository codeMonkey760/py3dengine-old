#ifndef PY3DENGINE_IMPORTERS_SCENE_H
#define PY3DENGINE_IMPORTERS_SCENE_H

#include <stdio.h>

struct ResourceManager;
struct Py3dGameObject;
extern void importScene(struct ResourceManager *manager, struct Py3dGameObject **rootPtr, FILE *sceneDescriptor);

#endif
