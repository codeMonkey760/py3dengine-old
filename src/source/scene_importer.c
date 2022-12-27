#include "logger.h"
#include "wfo_parser/wfo_parser.h"
#include "scene_importer.h"

static void initModel(struct Model **modelPtr, struct WfoParser *wfoParser, const char *name) {
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

    initModel(&curModel, wfoParser, "Cube");
    if (curModel == NULL) {
        error_log("%s", "[Scene Importer]: Unable to load \"Cube\" model");
    }
    storeModel(importer->manager, curModel);
    curModel = NULL;

    initModel(&curModel, wfoParser, "Pyramid");
    if (curModel == NULL) {
        error_log("%s", "[Scene Importer]: Unable to load \"Pyramid\" model");
    }
    storeModel(importer->manager, curModel);
    curModel = NULL;

    initModel(&curModel, wfoParser, "Quad");
    if (curModel == NULL) {
        error_log("%s", "[Scene Importer]: Unable to load \"Quad\" model");
    }
    storeModel(importer->manager, curModel);
    curModel = NULL;

    deleteWfoParser(&wfoParser);
    fclose(wfoFile);
}
