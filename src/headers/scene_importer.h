#ifndef PY3DENGINE_SCENE_IMPORTER_H
#define PY3DENGINE_SCENE_IMPORTER_H

#include <stdio.h>

struct GameObject;
struct ResourceManager;
struct FILE;

extern void importScene(
    struct ResourceManager *newManager,
    struct GameObject **rootPtr,
    FILE *sceneDescriptor
);

#endif
