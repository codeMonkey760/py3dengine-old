#ifndef PY3DENGINE_SCENE_IMPORTER_H
#define PY3DENGINE_SCENE_IMPORTER_H

#include <stdio.h>

#include "resource_manager.h"

struct SceneImporter {
    struct ResourceManager *manager;
};

extern void allocSceneImporter(struct SceneImporter **importerPtr);
extern void deleteSceneImporter(struct SceneImporter **importerPtr);

extern void initSceneImporter(struct SceneImporter *importer, struct ResourceManager *newManager);
extern void importScene(struct SceneImporter *importer, FILE *sceneDescriptor);

#endif
