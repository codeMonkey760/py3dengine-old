#include <json.h>

#include "logger.h"
#include "wfo_parser/wfo_parser.h"
#include "python/py3dgameobject.h"
#include "json_parser.h"
#include "resources/shader.h"
#include "resources/python_script.h"
#include "importers/texture.h"
#include "importers/shader.h"
#include "importers/component.h"
#include "importers/scene.h"
#include "importers/sprite_sheet.h"
#include "importers/builtins.h"
#include "python/py3dscene.h"
#include "python/py3dresourcemanager.h"

static const char *getResourceExt(const char *resourcePath) {
    if (resourcePath == NULL || (*resourcePath) == 0) return NULL;

    const char *curPos = resourcePath;
    while ((*curPos) != 0) curPos++;

    while (curPos != resourcePath && (*curPos) != '.') curPos--;

    return ((*curPos) == '.') ? curPos : NULL;
}

static void importResourceByDescriptor(struct Py3dResourceManager *manager, const char *resourcePath) {
    if (Py3dResourceManager_Check((PyObject *) manager) != 1 || resourcePath == NULL) return;

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
        Py3dResourceManager_StoreResource(manager, newTexture);
        newTexture = NULL;
    } else if (strcmp(typeName, "Shader") == 0) {
        struct BaseResource *newShader = NULL;
        importShader((struct Shader **) &newShader, resourceDescriptor);
        Py3dResourceManager_StoreResource(manager, newShader);
        newShader = NULL;
    } else if (strcmp(typeName, "Component") == 0) {
        struct BaseResource *newScript = NULL;
        importComponent((struct PythonScript **) &newScript, resourceDescriptor);
        Py3dResourceManager_StoreResource(manager, newScript);
        newScript = NULL;
    } else if (strcmp(typeName, "SpriteSheet") == 0) {
        importSprites(manager, resourceDescriptor);
    } else {
        error_log("[SceneImporter]: Could not identity resource type \"%s\"", typeName);
    }

    json_object_put(resourceDescriptor);
    resourceDescriptor = NULL;
}

static void importResourceByPath(struct Py3dResourceManager *manager, const char *resourcePath) {
    if (Py3dResourceManager_Check((PyObject *) manager) != 1 || resourcePath == NULL) return;

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

static void importResources(struct Py3dResourceManager *manager, json_object *resourceArray) {
    if (Py3dResourceManager_Check((PyObject *) manager) != 1 || resourceArray == NULL) return;

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

    json_object *scene_name = json_object_object_get(sceneDescriptor, "name");
    if (scene_name == NULL || !json_object_is_type(scene_name, json_type_string)) {
        PyErr_SetString(PyExc_ValueError, "Scene must have a string property called \"name\"");
        return NULL;
    }
    const char *scene_name_cstr = json_object_get_string(scene_name);

    trace_log("[SceneImporter]: Beginning scene import for \"%s\"", scene_name_cstr);

    struct Py3dScene *newScene = Py3dScene_New();
    if (newScene == NULL) return NULL;
    Py3dScene_SetNameCStr(newScene, scene_name_cstr);

    struct Py3dResourceManager *manager = Py3dResourceManager_New();

    Py3dScene_SetResourceManager(newScene, (PyObject *) manager);
    Py3dResourceManager_SetOwnerInC(manager, (struct Py3dScene *) newScene);

    importBuiltInResources(manager);

    json_object *resourceArray = json_object_object_get(sceneDescriptor, "resources");
    importResources(manager, resourceArray);

    json_object *scene_root = json_object_object_get(sceneDescriptor, "scene_root");
    if (scene_root == NULL || !json_object_is_type(scene_root, json_type_object)) {
        PyErr_SetString(PyExc_ValueError, "Scene must have an object property called \"scene_root\"");
        Py_CLEAR(newScene);
        return NULL;
    }

    struct Py3dGameObject *rootGO = NULL;
    if (!parseGameObject(scene_root, NULL, &rootGO, newScene, manager)) {
        PyErr_SetString(PyExc_ValueError, "Unable to parse scene");
        Py_CLEAR(newScene);
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

    trace_log("[SceneImporter]: Scene import for \"%s\" has ended", scene_name_cstr);

    return newScene;
}
