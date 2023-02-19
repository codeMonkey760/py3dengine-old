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

void importScene(struct ResourceManager *manager, struct Py3dGameObject **rootPtr, FILE *sceneDescriptor) {
    if (manager == NULL || rootPtr == NULL || (*rootPtr) != NULL || sceneDescriptor == NULL) return;

    json_object *json_root = json_object_from_fd(fileno(sceneDescriptor));
    if (json_root == NULL) {
        critical_log("%s", "[SceneImporter]: Could not parse scene descriptor");
        return;
    }

    struct PythonScript *script = NULL;
    importBuiltinComponent(&script, "ModelRendererComponent");
    if (script == NULL) {
        error_log("%s", "[SceneImporter]: Unable to load ModelRendererComponent builtin");
    }
    storeResource(manager, (struct BaseResource *) script);
    script = NULL;

    json_object *resourceArray = json_object_object_get(json_root, "resources");
    importResources(manager, resourceArray);

    json_object *scene_root = json_object_object_get(json_root, "scene_root");
    if (scene_root == NULL || !json_object_is_type(scene_root, json_type_object)) {
        critical_log("%s", "[SceneImporter]: Scene must have an object property called \"scene_root\"");
        json_object_put(json_root);
        return;
    }

    struct Py3dGameObject *rootGO = NULL;
    parseGameObject(scene_root, NULL, &rootGO, manager);

    (*rootPtr) = rootGO;
    rootGO = NULL;

    json_object_put(json_root);
    json_root = NULL;
}
