#include "logger.h"
#include "config.h"
#include "wfo_parser/wfo_parser.h"
#include "scene_importer.h"
#include "game_object.h"
#include "components/model_renderer_component.h"
#include "components/rotation_component.h"
#include "components/transform_component.h"
#include "components/camera_component.h"

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

void initSceneImporter(struct SceneImporter *importer, struct ResourceManager *newManager, struct GameObject **rootPtr) {
    if (importer == NULL) return;

    importer->manager = newManager;
    importer->sceneRootPtr = rootPtr;
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

    FILE *mtlFile = fopen("resources/solid_objs.mtl", "r");
    if (mtlFile == NULL) {
        error_log("%s", "[SceneImporter]: Could not open \"solid_objs.mtl\". Material parsing will fail.");
    } else {
        parseMaterialFile(importer->manager, mtlFile);
    }
    fclose(mtlFile);

    deleteWfoParser(&wfoParser);
    fclose(wfoFile);

    if (importer->sceneRootPtr == NULL || (*importer->sceneRootPtr) != NULL) return;

    struct GameObject *root = NULL, *curGO = NULL;
    struct ModelRendererComponent *curMRC = NULL;
    struct RotationComponent *curRC = NULL;
    struct CameraComponent *curCC = NULL;
    float posW[3] = {0.0f, 0.0f, 2.0f};
    float axis[3] = {0.0f};

    // Root Game Object
    allocGameObject(&root);
    setGameObjectName(root, "Root");

    // The Cube
    allocGameObject(&curGO);
    setGameObjectName(curGO, "Cube");

    posW[0] = -3.0f;
    setTransformPosition(getGameObjectTransform(curGO), posW);
    posW[0] = 0.0f;

    allocModelRendererComponent(&curMRC);
    setComponentName((struct BaseComponent *) curMRC, "Cube.MRC");
    setModelRendererComponentModel(curMRC, getModelResource(importer->manager, "Cube"));
    setModelRendererComponentShader(curMRC, getShaderResource(importer->manager, "SolidColorShader"));
    setModelRendererComponentMaterial(curMRC, getMaterialResource(importer->manager, "SolidBlue"));
    attachComponent(curGO, (struct BaseComponent *) curMRC);
    curMRC = NULL;

    allocRotationComponent(&curRC);
    setComponentName((struct BaseComponent *) curRC, "Cube.RC");
    axis[0] = 1.0f;
    setRotationComponentAxis(curRC, axis);
    axis[0] = 0.0f;
    setRotationComponentSpeed(curRC, 45.0f);
    attachComponent(curGO, (struct BaseComponent *) curRC);
    curRC = NULL;

    attachChild(root, curGO);
    curGO = NULL;

    // The Pyramid
    allocGameObject(&curGO);
    setGameObjectName(curGO, "Pyramid");

    setTransformPosition(getGameObjectTransform(curGO), posW);

    allocModelRendererComponent(&curMRC);
    setComponentName((struct BaseComponent *) curMRC, "Pyramid.MRC");
    setModelRendererComponentModel(curMRC, getModelResource(importer->manager, "Pyramid"));
    setModelRendererComponentShader(curMRC, getShaderResource(importer->manager, "SolidColorShader"));
    setModelRendererComponentMaterial(curMRC, getMaterialResource(importer->manager, "SolidYellow"));
    attachComponent(curGO, (struct BaseComponent *) curMRC);
    curMRC = NULL;

    allocRotationComponent(&curRC);
    setComponentName((struct BaseComponent *) curRC, "Pyramid.RC");
    axis[1] = 1.0f;
    setRotationComponentAxis(curRC, axis);
    axis[1] = 0.0f;
    setRotationComponentSpeed(curRC, 25.0f);
    attachComponent(curGO, (struct BaseComponent *) curRC);
    curRC = NULL;

    attachChild(root, curGO);
    curGO = NULL;

    // The Quad
    allocGameObject(&curGO);
    setGameObjectName(curGO, "Quad");

    posW[0] = 3.0f;
    setTransformPosition(getGameObjectTransform(curGO), posW);
    posW[0] = 0.0f;

    allocModelRendererComponent(&curMRC);
    setComponentName((struct BaseComponent *) curMRC, "Quad.MRC");
    setModelRendererComponentModel(curMRC, getModelResource(importer->manager, "Quad"));
    setModelRendererComponentShader(curMRC, getShaderResource(importer->manager, "SolidColorShader"));
    setModelRendererComponentMaterial(curMRC, getMaterialResource(importer->manager, "SolidRed"));
    attachComponent(curGO, (struct BaseComponent *) curMRC);
    curMRC = NULL;

    allocRotationComponent(&curRC);
    setComponentName((struct BaseComponent *) curRC, "Quad.RC");
    axis[2] = 1.0f;
    setRotationComponentAxis(curRC, axis);
    axis[2] = 0.0f;
    setRotationComponentSpeed(curRC, 90.0f);
    attachComponent(curGO, (struct BaseComponent *) curRC);
    curRC = NULL;

    attachChild(root, curGO);
    curGO = NULL;

    // The Camera
    allocGameObject(&curGO);
    setGameObjectName(curGO, "Camera");

    posW[2] = -2.0f;
    setTransformPosition(getGameObjectTransform(curGO), posW);
    posW[2] = 0.0f;

    allocCameraComponent(&curCC);
    setComponentName((struct BaseComponent *) curCC, "Camera.CC");
    setCameraComponentLens(curCC, 100.0f, getConfigScreenWidth(), getConfigScreenHeight(), 0.05f, 10.0f);
    attachComponent(curGO, (struct BaseComponent *) curCC);
    curCC = NULL;

    attachChild(root, curGO);
    curGO = NULL;

    (*importer->sceneRootPtr) = root;
}
