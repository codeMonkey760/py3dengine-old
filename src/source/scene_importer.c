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
#include "python/python_util.h"
#include "python/py3denginemodule.h"

static void importShader(struct Shader **shaderPtr, const char *fileName) {
    if (shaderPtr == NULL || (*shaderPtr) != NULL || fileName == NULL) return;

    json_object *shader_desc_root = json_object_from_file(fileName);

    json_object *json_type_name = json_object_object_get(shader_desc_root, "type");
    if (json_type_name == NULL || !json_object_is_type(json_type_name, json_type_string)) {
        error_log("%s", "[SceneImporter]: Shader description must have an attribute of type \"string\" called \"type\"");
        return;
    }
    if (strcmp(json_object_get_string(json_type_name), "shader") != 0) {
        error_log("%s", "[SceneImporter]: Shader description type attribute must be set to \"shader\"");
        return;
    }

    json_object *json_name = json_object_object_get(shader_desc_root, "name");
    if (json_name == NULL || !json_object_is_type(json_name, json_type_string)) {
        error_log("%s", "[SceneImporter]: Shader description must have an attribute of type \"string\" called \"name\"");
        return;
    }

    json_object *vertex_shader_file_name = json_object_object_get(shader_desc_root, "vertex_shader_source_file");
    if (vertex_shader_file_name == NULL || !json_object_is_type(vertex_shader_file_name, json_type_string)) {
        error_log("%s", "[SceneImporter]: Shader description must have an attribute of type \"string\" called \"vertex_shader_source_file\"");
        return;
    }

    json_object *fragment_shader_file_name = json_object_object_get(shader_desc_root, "fragment_shader_source_file");
    if (fragment_shader_file_name == NULL || !json_object_is_type(fragment_shader_file_name, json_type_string)) {
        error_log("%s", "[SceneImporter]: Shader description must have an attribute of type \"string\" called \"fragment_shader_source_file\"");
        return;
    }

    struct Shader *newShader = NULL;
    allocShader(&newShader);
    if (newShader == NULL) return;

    initShaderFromFiles(
        newShader,
        json_object_get_string(vertex_shader_file_name),
        json_object_get_string(fragment_shader_file_name)
    );
    setResourceName((struct BaseResource *) newShader, json_object_get_string(json_name));

    (*shaderPtr) = newShader;
    newShader = NULL;

    json_object_put(shader_desc_root);
    shader_desc_root = NULL;
}

static void importTestScript(struct PythonScript **scriptPtr, const char *name) {
    if (scriptPtr == NULL || (*scriptPtr) != NULL) return;

    PyObject *componentModule = PyImport_ImportModule(name);
    if (componentModule == NULL) {
        handleException();
        return;
    }

    struct PythonScript *newScript = NULL;
    allocPythonScript(&newScript);
    if (newScript == NULL) return;

    initPythonScript(newScript, componentModule, name);
    setResourceName((struct BaseResource *) newScript, name);

    (*scriptPtr) = newScript;
    newScript = NULL;
}

static void importBuiltinComponent(struct PythonScript **scriptPtr, const char *name) {
    if (scriptPtr == NULL || (*scriptPtr) != NULL) return;

    PyObject *py3dEngineModule = getPy3dEngineModule();
    if (py3dEngineModule == NULL) return;

    struct PythonScript *newScript = NULL;
    allocPythonScript(&newScript);
    if (newScript == NULL) return;

    initPythonScript(newScript, py3dEngineModule, name);
    setResourceName((struct BaseResource *) newScript, name);

    (*scriptPtr) = newScript;
    newScript = NULL;
}

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

    FILE *wfoFile = fopen("resources/solid_objs.obj", "r");
    if (wfoFile == NULL) {
        error_log("%s", "[Scene Importer]: Could not open \"solid_objs.obj\" wfo file");
        return;
    } else {
        parseWaveFrontFile(importer->manager, wfoFile);
    }
    fclose(wfoFile);

    FILE *mtlFile = fopen("resources/solid_objs.mtl", "r");
    if (mtlFile == NULL) {
        error_log("%s", "[SceneImporter]: Could not open \"solid_objs.mtl\". Material parsing will fail.");
    } else {
        parseMaterialFile(importer->manager, mtlFile);
    }
    fclose(mtlFile);

    struct Shader *curShader = NULL;
    importShader(&curShader, "resources/general_pnt.json");
    if (curShader == NULL) {
        error_log("%s", "[Scene Importer]: Unable to load \"general_pnt\" shader");
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

    importTestScript(&script, "RotationComponent");
    if (script == NULL) {
        error_log("%s", "[SceneImporter]: Unable to load RotationComponent python script");
    }
    storeResource(importer->manager, (struct BaseResource *) script);
    script = NULL;

    importTestScript(&script, "CameraComponent");
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
