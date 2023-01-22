#include <json-c/json.h>

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

static const char *vertex_shader_source =
        "#version 460 core\n\n"

        "layout(location = 0) in vec3 posL;\n"
        "layout(location = 1) in vec3 normL;\n\n"

        "uniform mat4 gWMtx;\n"
        "uniform mat4 gWITMtx;\n"
        "uniform mat4 gWVPMtx;\n\n"

        "out vec3 posW;\n"
        "out vec3 normW;\n\n"

        "void main() {\n"
        "    posW = (vec4(posL, 1.0f) * gWMtx).xyz;\n"
        "    normW = (vec4(normL, 0.0f) * gWITMtx).xyz;\n\n"

        "    gl_Position = (vec4(posL, 1.0) * gWVPMtx);\n"
        "}\n";

static const char *fragment_shader_source =
        "#version 460 core\n\n"

        "in vec3 posW;\n"
        "in vec3 normW;\n\n"

        "uniform vec3 gDiffuseColor;\n"
        "uniform vec3 gCamPos;\n\n"

        "layout(location = 0) out vec4 outputColor;\n\n"

        "void main() {\n"
        "    vec3 normWFixed = normalize(normW);\n"
        "    vec3 toCamera = normalize(gCamPos - posW);\n\n"

        "    float lightValue = max(dot(toCamera, normWFixed), 0.0f);\n"
        "    lightValue = (lightValue * 0.7f) + 0.3f;\n\n"

        "    outputColor = vec4(gDiffuseColor * lightValue, 1.0f);\n"
        "}\n";

static void importModel(struct Model **modelPtr, struct WfoParser *wfoParser, const char *name) {
    if (modelPtr == NULL || (*modelPtr) != NULL || wfoParser == NULL || name == NULL) return;

    unsigned long cubeVboSize = getUnIndexedVertexBufferSizeInFloats(wfoParser, name);
    if (cubeVboSize == 0) return;

    debug_log("Allocating %d * %d = %d for VBO with name %s",
              cubeVboSize, sizeof(float),
              cubeVboSize * sizeof(float),
              name
    );
    float *vbo = calloc(cubeVboSize, sizeof(float));
    if (vbo == NULL) return;

    getUnIndexedVertexBuffer(wfoParser, name, vbo, cubeVboSize);

    struct Model *newModel = NULL;
    allocModel(&newModel);
    if (newModel == NULL) {
        free(vbo);
        vbo = NULL;

        return;
    }

    setPNTBuffer(newModel, vbo, cubeVboSize / 8);
    setResourceName((struct BaseResource *) newModel, name);

    free(vbo);
    vbo = NULL;

    (*modelPtr) = newModel;
    newModel = NULL;
}

static void importShader(struct Shader **shaderPtr) {
    if (shaderPtr == NULL || (*shaderPtr) != NULL) return;

    struct Shader *newShader = NULL;
    allocShader(&newShader);
    if (newShader == NULL) return;

    initShader(newShader, vertex_shader_source, fragment_shader_source);
    setResourceName((struct BaseResource *) newShader, "SolidColorShader");

    (*shaderPtr) = newShader;
    newShader = NULL;
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
        error_log("%s", "[Scene Importer]: Could not open solid_objs.obj wfo file");
        return;
    }

    struct WfoParser *wfoParser = NULL;
    allocWfoParser(&wfoParser);
    parseWaveFrontFile(wfoParser, wfoFile);

    struct Model *curModel = NULL;

    importModel(&curModel, wfoParser, "Cube");
    if (curModel == NULL) {
        error_log("%s", "[Scene Importer]: Unable to load \"Cube\" model");
    }
    storeResource(importer->manager, (struct BaseResource *) curModel);
    curModel = NULL;

    importModel(&curModel, wfoParser, "Pyramid");
    if (curModel == NULL) {
        error_log("%s", "[Scene Importer]: Unable to load \"Pyramid\" model");
    }
    storeResource(importer->manager, (struct BaseResource *) curModel);
    curModel = NULL;

    importModel(&curModel, wfoParser, "Quad");
    if (curModel == NULL) {
        error_log("%s", "[Scene Importer]: Unable to load \"Quad\" model");
    }
    storeResource(importer->manager, (struct BaseResource *) curModel);
    curModel = NULL;

    struct Shader *curShader = NULL;

    importShader(&curShader);
    if (curShader == NULL) {
        error_log("%s", "[Scene Importer]: Unable to load \"SolidColorShader\" shader");
    }
    storeResource(importer->manager, (struct BaseResource *) curShader);
    curShader = NULL;

    FILE *mtlFile = fopen("resources/solid_objs.mtl", "r");
    if (mtlFile == NULL) {
        error_log("%s", "[SceneImporter]: Could not open \"solid_objs.mtl\". Material parsing will fail.");
    } else {
        parseMaterialFile(importer->manager, mtlFile);
    }
    fclose(mtlFile);

    deleteWfoParser(&wfoParser);
    fclose(wfoFile);

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
