#ifndef PY3DENGINE_SCENE_IMPORTER_H
#define PY3DENGINE_SCENE_IMPORTER_H

#include <stdio.h>

struct SceneImporter {
};

extern void allocSceneImporter(struct SceneImporter **importerPtr);
extern void deleteSceneImporter(struct SceneImporter **importPtr);

extern void initSceneImporter(struct SceneImporter *importer, struct ResourceManager *manager);
extern void importScene(struct SceneImporter *importer, FILE *sceneDescriptor);

#endif
