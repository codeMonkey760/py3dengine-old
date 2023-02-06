#ifndef PY3DENGINE_IMPORTERS_SCENE_H
#define PY3DENGINE_IMPORTERS_SCENE_H

#include <stdio.h>

struct ResourceManager;
struct GameObject;
extern void importScene(struct ResourceManager *manager, struct GameObject **rootPtr, FILE *sceneDescriptor);

#endif
