#include <json.h>

#include "logger.h"
#include "wfo_parser/wfo_parser.h"
#include "python/py3dgameobject.h"
#include "json_parser.h"
#include "resources/shader.h"
#include "resources/python_script.h"
#include "resource_manager.h"
#include "importers/texture.h"
#include "importers/shader.h"
#include "importers/component.h"
#include "importers/scene.h"
#include "importers/sprite_sheet.h"
#include "importers/builtins.h"
#include "python/py3dscene.h"

static const char *getResourceExt(const char *resourcePath) {
    if (resourcePath == NULL || (*resourcePath) == 0) return NULL;

    const char *curPos = resourcePath;
    while ((*curPos) != 0) curPos++;

    while (curPos != resourcePath && (*curPos) != '.') curPos--;

    return ((*curPos) == '.') ? curPos : NULL;
}

static void importResourceByDescriptor(struct ResourceManager *manager, const char *resourcePath) {
    if (manager == NULL || resourcePath == NULL) return;

    json_object *resourceDescriptor = json_object_from_file(resourcePath);
    if (resourceDescriptor == NULL) {
        error_log("[SceneImporter]: Could not parse json from \"%s\"", resourcePath);
        return;
    }

    json_object *json_type = json_object_object_get(resourceDescriptor, "type");
    if (json_type == NULL || !json_object_is_type(json_type, json_type_string)) {
        error_log("[SceneImporter]: Resource descriptor must have a \"type\" field of type string");
        json_object_put(resourceDescriptor);
        resourceDescriptor = NULL;
        return;
    }

    const char *typeName = json_object_get_string(json_type);
    if (strcmp(typeName, "Texture") == 0) {
        struct BaseResource *newTexture = NULL;
        importTexture((struct Texture **) &newTexture, resourceDescriptor);
        storeResource(manager, newTexture);
        newTexture = NULL;
    } else if (strcmp(typeName, "Shader") == 0) {
        struct BaseResource *newShader = NULL;
        importShader((struct Shader **) &newShader, resourceDescriptor);
        storeResource(manager, newShader);
        newShader = NULL;
    } else if (strcmp(typeName, "Component") == 0) {
        struct BaseResource *newScript = NULL;
        importComponent((struct PythonScript **) &newScript, resourceDescriptor);
        storeResource(manager, newScript);
        newScript = NULL;
    } else if (strcmp(typeName, "SpriteSheet") == 0) {
        importSprites(manager, resourceDescriptor);
    } else {
        error_log("[SceneImporter]: Could not identity resource type \"%s\"", typeName);
    }

    json_object_put(resourceDescriptor);
    resourceDescriptor = NULL;
}

static void importResourceByPath(struct ResourceManager *manager, const char *resourcePath) {
    const char *ext = getResourceExt(resourcePath);
    if (ext == NULL) return;

    if (strcmp(ext, ".obj") == 0) {
        importWaveFrontFile(manager, resourcePath);
    } else if (strcmp(ext, ".mtl") == 0) {
        importMaterialFile(manager, resourcePath);
    } else if (strcmp(ext, ".json") == 0) {
        importResourceByDescriptor(manager, resourcePath);
    } else {
        error_log("[SceneImporter]: Unable to determine resource type \"%s\"", resourcePath);
    }
}

static void importResources(struct ResourceManager *manager, json_object *resourceArray) {
    if (manager == NULL || resourceArray == NULL) return;

    if (!json_object_is_type(resourceArray, json_type_array)) {
        error_log("%s", "[SceneImporter]: \"resources\" field must be of type array");
        return;
    }

    size_t resourceCount = json_object_array_length(resourceArray);
    for (size_t i = 0; i < resourceCount; ++i) {
        json_object *curResourceName = json_object_array_get_idx(resourceArray, i);
        if (curResourceName == NULL || !json_object_is_type(curResourceName, json_type_string)) {
            error_log("%s", "[SceneImporter]: Resources names must be of type string");
            continue;
        }

        importResourceByPath(manager, json_object_get_string(curResourceName));
    }
}

struct Py3dScene *importScene(json_object *sceneDescriptor) {
    if (sceneDescriptor == NULL) {
        PyErr_SetString(PyExc_ValueError, "Scene descriptor must provide valid JSON");
        return NULL;
    }

    struct Py3dScene *newScene = Py3dScene_New();
    if (newScene == NULL) return NULL;

    struct ResourceManager *manager = NULL;
    allocResourceManager(&manager);

    Py_INCREF(manager->py3dResourceManager);
    Py3dScene_SetResourceManager(newScene, (PyObject *) manager->py3dResourceManager);
    setResourceManagerOwner(manager, (struct Py3dScene *) newScene);

    importBuiltInResources(manager);

    json_object *resourceArray = json_object_object_get(sceneDescriptor, "resources");
    importResources(manager, resourceArray);

    json_object *scene_root = json_object_object_get(sceneDescriptor, "scene_root");
    if (scene_root == NULL || !json_object_is_type(scene_root, json_type_object)) {
        PyErr_SetString(PyExc_ValueError, "Scene must have an object property called \"scene_root\"");
        deleteResourceManager(&manager);
        return NULL;
    }

    struct Py3dGameObject *rootGO = NULL;
    if (!parseGameObject(scene_root, NULL, &rootGO, newScene, manager)) {
        PyErr_SetString(PyExc_ValueError, "Unable to parse scene");
        deleteResourceManager(&manager);
        Py_CLEAR(rootGO);
        return NULL;
    }

    Py3dScene_SetSceneGraph(newScene, (PyObject *) rootGO);
    rootGO = NULL;

    PyObject *activateCameraRet = Py3dScene_ActivateCameraByNameCStr(newScene, "Camera");
    if (activateCameraRet == NULL) {
        Py_CLEAR(newScene);
        // rootGO and manager are owned by newScene, clean up unnecessary
        return NULL;
    }
    Py_CLEAR(activateCameraRet);

    return newScene;
}
