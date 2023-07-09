#ifndef PY3DENGINE_IMPORTERS_SCENE_H
#define PY3DENGINE_IMPORTERS_SCENE_H

#include <stdio.h>

struct Py3dScene;
extern const char *peekSceneName(json_object *sceneDescriptor);
extern struct Py3dScene *importScene(json_object *sceneDescriptor);

#endif
