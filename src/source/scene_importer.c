#include <json.h>

#include "logger.h"
#include "wfo_parser/wfo_parser.h"
#include "scene_importer.h"
#include "game_object.h"
#include "json_parser.h"
#include "resources/model.h"
#include "resources/shader.h"
#include "resources/python_script.h"
#include "resource_manager.h"
#include "importers/texture.h"
#include "importers/shader.h"
#include "importers/component.h"

void allocSceneImporter(struct SceneImporter **importerPtr) {
    if (importerPtr == NULL || (*importerPtr) != NULL) return;

    struct SceneImporter *newImporter = calloc(1, sizeof(struct SceneImporter));
    if (newImporter == NULL) return;

    newImporter->manager = NULL;

    (*importerPtr) = newImporter;
    newImporter = NULL;
}

void deleteSceneImporter(struct SceneImporter **importerPtr) {
    if (importerPtr == NULL || (*importerPtr) == NULL) return;

    struct SceneImporter *importer = (*importerPtr);
    importer->manager = NULL;

    free(importer);
    importer = NULL;
    (*importerPtr) = NULL;
}

void initSceneImporter(struct SceneImporter *importer, struct ResourceManager *newManager, struct GameObject **rootPtr) {
    if (importer == NULL) return;

    importer->manager = newManager;
    importer->sceneRootPtr = rootPtr;
}

void importScene(struct SceneImporter *importer, FILE *sceneDescriptor) {
    if (importer == NULL || importer->manager == NULL || sceneDescriptor == NULL) return;

    struct Texture *curTexture = NULL;
    importTexture(&curTexture, "resources/test_pattern_point.json");
    if (curTexture == NULL) {
        error_log("%s", "[SceneImporter]: Could not load \"test_pattern_point\" as texture. Material parsing will fail.");
    } else {
        storeResource(importer->manager, (struct BaseResource *) curTexture);
        curTexture = NULL;
    }

    importTexture(&curTexture, "resources/test_pattern_linear.json");
    if (curTexture == NULL) {
        error_log("%s", "[SceneImporter]: Could not load \"test_pattern_linear\" as texture. Material parsing will fail.");
    } else {
        storeResource(importer->manager, (struct BaseResource *) curTexture);
        curTexture = NULL;
    }

    FILE *wfoFile = fopen("resources/solid_objs.obj", "r");
    if (wfoFile == NULL) {
        error_log("%s", "[SceneImporter]: Could not open \"solid_objs.obj\" wfo file");
        return;
    } else {
        parseWaveFrontFile(importer->manager, wfoFile);
    }
    fclose(wfoFile);

    FILE *mtlFile = fopen("resources/solid_objs.mtl", "r");
    if (mtlFile == NULL) {
        error_log("%s", "[SceneImporter]: Could not open \"solid_objs.mtl\". Material parsing has failed.");
    } else {
        parseMaterialFile(importer->manager, mtlFile);
    }
    fclose(mtlFile);

    struct Shader *curShader = NULL;
    importShader(&curShader, "resources/general_pnt.json");
    if (curShader == NULL) {
        error_log("%s", "[SceneImporter]: Unable to load \"general_pnt\" shader");
    }
    storeResource(importer->manager, (struct BaseResource *) curShader);
    curShader = NULL;

    struct PythonScript *script = NULL;
    importBuiltinComponent(&script, "ModelRendererComponent");
    if (script == NULL) {
        error_log("%s", "[SceneImporter]: Unable to load ModelRendererComponent builtin");
    }
    storeResource(importer->manager, (struct BaseResource *) script);
    script = NULL;

    importComponent(&script, "RotationComponent");
    if (script == NULL) {
        error_log("%s", "[SceneImporter]: Unable to load RotationComponent python script");
    }
    storeResource(importer->manager, (struct BaseResource *) script);
    script = NULL;

    importComponent(&script, "CameraComponent");
    if (script == NULL) {
        error_log("%s", "[SceneImporter]: Unable to load CameraComponent python script");
    }
    storeResource(importer->manager, (struct BaseResource *) script);
    script = NULL;

    if (importer->sceneRootPtr == NULL || (*importer->sceneRootPtr) != NULL) return;

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
    parseGameObject(scene_root, NULL, &rootGO, importer->manager);

    (*importer->sceneRootPtr) = rootGO;

    json_object_put(json_root);
}
