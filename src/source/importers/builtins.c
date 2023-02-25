#include "importers/builtins.h"
#include "importers/component.h"
#include "resource_manager.h"
#include "logger.h"
#include "resources/model.h"

static void importQuadModel(struct ResourceManager *rm) {
    if (rm == NULL) return;

    struct Model *quad = NULL;
    allocModel(&quad);
    if (quad == NULL) {
        critical_log("%s", "[BuiltInImporter]: Unable to load Quad Model builtin");
        return;
    }
    setResourceName((struct BaseResource *) quad, "QuadModelBuiltIn");

    struct VertexPNT vertices[] = {
        {-1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
        {-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
        { 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f},

        { 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f},
        { 1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
        {-1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}
    };

    setModelPNTBuffer(quad, vertices, 6);

    storeResource(rm, (struct BaseResource *) quad);
    quad = NULL;
}

void importBuiltInResources(struct ResourceManager *rm) {
    if (rm == NULL) return;

    struct PythonScript *script = NULL;
    importBuiltinComponent(&script, "ModelRendererComponent");
    if (script == NULL) {
        error_log("%s", "[BuiltInImporter]: Unable to load ModelRendererComponent builtin");
    }
    storeResource(rm, (struct BaseResource *) script);
    script = NULL;

    importBuiltinComponent(&script, "SpriteRendererComponent");
    if (script == NULL) {
        error_log("%s", "[BuiltInImporter]: Unable to load SpriteRendererComponent builtin");
    }
    storeResource(rm, (struct BaseResource *) script);
    script = NULL;

    importQuadModel(rm);
}
