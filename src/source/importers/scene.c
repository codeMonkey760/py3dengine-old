#include <json.h>

#include "logger.h"
#include "wfo_parser/wfo_parser.h"
#include "game_object.h"
#include "json_parser.h"
#include "resources/shader.h"
#include "resources/python_script.h"
#include "resource_manager.h"
#include "importers/texture.h"
#include "importers/shader.h"
#include "importers/component.h"
#include "importers/scene.h"

// TODO: make this not hard coded
void importScene(struct ResourceManager *manager, struct GameObject **rootPtr, FILE *sceneDescriptor) {
    if (manager == NULL || rootPtr == NULL || (*rootPtr) != NULL || sceneDescriptor == NULL) return;

    struct Texture *curTexture = NULL;
    importTexture(&curTexture, "resources/test_pattern_point.json");
    if (curTexture == NULL) {
        error_log("%s", "[SceneImporter]: Could not load \"test_pattern_point\" as texture. Material parsing will fail.");
    } else {
        storeResource(manager, (struct BaseResource *) curTexture);
        curTexture = NULL;
    }

    importTexture(&curTexture, "resources/test_pattern_linear.json");
    if (curTexture == NULL) {
        error_log("%s", "[SceneImporter]: Could not load \"test_pattern_linear\" as texture. Material parsing will fail.");
    } else {
        storeResource(manager, (struct BaseResource *) curTexture);
        curTexture = NULL;
    }

    FILE *wfoFile = fopen("resources/solid_objs.obj", "r");
    if (wfoFile == NULL) {
        error_log("%s", "[SceneImporter]: Could not open \"solid_objs.obj\" wfo file");
        return;
    } else {
        parseWaveFrontFile(manager, wfoFile);
    }
    fclose(wfoFile);

    FILE *mtlFile = fopen("resources/solid_objs.mtl", "r");
    if (mtlFile == NULL) {
        error_log("%s", "[SceneImporter]: Could not open \"solid_objs.mtl\". Material parsing has failed.");
    } else {
        parseMaterialFile(manager, mtlFile);
    }
    fclose(mtlFile);

    struct Shader *curShader = NULL;
    importShader(&curShader, "resources/general_pnt.json");
    if (curShader == NULL) {
        error_log("%s", "[SceneImporter]: Unable to load \"general_pnt\" shader");
    }
    storeResource(manager, (struct BaseResource *) curShader);
    curShader = NULL;

    struct PythonScript *script = NULL;
    importBuiltinComponent(&script, "ModelRendererComponent");
    if (script == NULL) {
        error_log("%s", "[SceneImporter]: Unable to load ModelRendererComponent builtin");
    }
    storeResource(manager, (struct BaseResource *) script);
    script = NULL;

    importComponent(&script, "RotationComponent");
    if (script == NULL) {
        error_log("%s", "[SceneImporter]: Unable to load RotationComponent python script");
    }
    storeResource(manager, (struct BaseResource *) script);
    script = NULL;

    importComponent(&script, "CameraComponent");
    if (script == NULL) {
        error_log("%s", "[SceneImporter]: Unable to load CameraComponent python script");
    }
    storeResource(manager, (struct BaseResource *) script);
    script = NULL;

    json_object *json_root = json_object_from_fd(fileno(sceneDescriptor));
    if (json_root == NULL) {
        critical_log("%s", "[SceneImporter]: Could not parse the contents of \"default.json\" as json. Scene loading will fail.");
        return;
    }

    json_object *scene_root = json_object_object_get(json_root, "scene_root");
    if (scene_root == NULL || !json_object_is_type(scene_root, json_type_object)) {
        critical_log("%s", "[SceneImporter]: Scene must have an object property called \"scene_root\"");
        json_object_put(json_root);
        return;
    }

    struct GameObject *rootGO = NULL;
    parseGameObject(scene_root, NULL, &rootGO, manager);

    (*rootPtr) = rootGO;
    rootGO = NULL;

    json_object_put(json_root);
    json_root = NULL;
}
