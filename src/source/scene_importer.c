#include "logger.h"
#include "wfo_parser/wfo_parser.h"
#include "scene_importer.h"

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
    setModelName(newModel, name);

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
    setShaderName(newShader, "SolidColorShader");

    (*shaderPtr) = newShader;
    newShader = NULL;
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

void initSceneImporter(struct SceneImporter *importer, struct ResourceManager *newManager) {
    if (importer == NULL) return;

    importer->manager = newManager;
}

void importScene(struct SceneImporter *importer, FILE *sceneDescriptor) {
    // TODO: add a NULL check for sceneDescriptor when I start using it
    if (importer == NULL || importer->manager == NULL) return;

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
    storeModel(importer->manager, curModel);
    curModel = NULL;

    importModel(&curModel, wfoParser, "Pyramid");
    if (curModel == NULL) {
        error_log("%s", "[Scene Importer]: Unable to load \"Pyramid\" model");
    }
    storeModel(importer->manager, curModel);
    curModel = NULL;

    importModel(&curModel, wfoParser, "Quad");
    if (curModel == NULL) {
        error_log("%s", "[Scene Importer]: Unable to load \"Quad\" model");
    }
    storeModel(importer->manager, curModel);
    curModel = NULL;

    struct Shader *curShader = NULL;

    importShader(&curShader);
    if (curShader == NULL) {
        error_log("%s", "[Scene Importer]: Unable to load \"SolidColorShader\" shader");
    }
    storeShader(importer->manager, curShader);
    curShader = NULL;

    deleteWfoParser(&wfoParser);
    fclose(wfoFile);
}
