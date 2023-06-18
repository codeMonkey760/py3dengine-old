#ifndef PY3DENGINE_IMPORTERS_SCENE_H
#define PY3DENGINE_IMPORTERS_SCENE_H

#include <stdio.h>

struct Py3dScene;

extern struct Py3dScene *importScene(json_object *sceneDescriptor);

#endif
