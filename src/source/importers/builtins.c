#include "importers/builtins.h"
#include "importers/component.h"
#include "resource_manager.h"
#include "logger.h"
#include "resources/model.h"
#include "resources/shader.h"

const char *spriteVertexShader =
"#version 460 core\n"
"\n"
"layout(location = 0) in vec3 posL;\n"
"layout(location = 2) in vec2 inTexC;\n"
"\n"
"uniform mat4 gWVPMtx;\n"
"uniform mat3 gTexMtx;\n"
"\n"
"out vec2 texCoord;\n"
"\n"
"void main() {\n"
"   texCoord = (vec3(inTexC, 1.0) * gTexMtx).xy;\n"
"   gl_Position = (vec4(posL, 1.0) * gWVPMtx);\n"
"}\n";

const char *spriteFragShader =
"#version 460 core\n"
"\n"
"in vec2 texCoord;\n"
"\n"
"uniform vec3 gMixColor;\n"
"uniform sampler2D gSprite;\n"
"\n"
"layout(location = 0) out vec4 outputColor;\n"
"\n"
"void main() {\n"
"   outputColor = vec4(texture(gSprite, texCoord).rgb * gMixColor, 1.0);"
"}\n";

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

static void importSpriteShader(struct ResourceManager *rm) {
    if (rm == NULL) return;

    struct Shader *shader = NULL;
    allocShader(&shader);
    if (shader == NULL) {
        critical_log("%s", "[BuiltInImporter]: Unable to allocate Sprite Shader");
        return;
    }

    setResourceName((struct BaseResource *) shader, "SpriteShader");
    initShader(shader, spriteVertexShader, spriteFragShader);

    storeResource(rm, (struct BaseResource *) shader);
    shader = NULL;
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
