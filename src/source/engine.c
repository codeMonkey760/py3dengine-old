#include <stdlib.h>

#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include "logger.h"
#include "util.h"
#include "shader.h"
#include "engine.h"

#include "wfo_parser/wfo_parser.h"

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


static void error_callback(int code, const char* description) {
    error_log("%s 0x%x %s\n", "GLFW error code", code, description);
}

static void updateStats(struct Engine *engine, float dt) {
    static float since_last_calc = 0.0f;
    static int frame_count = 0;

    if (engine == NULL) return;

    engine->_elapsed_time += dt;
    frame_count++;
    since_last_calc += dt;

    if (engine->_print_report == true && since_last_calc >= 1.0f) {
        float ms = (since_last_calc / (float) frame_count) * 1000.0f;
        float fps = (float) frame_count;

        frame_count = 0;
        while (since_last_calc >= 1.0f) {
            since_last_calc -= 1.0f;
        }

        engine->_fps = fps;
        engine->_mpf = ms;
    }
}

static void printStats(struct Engine *engine, float dt) {
    if (engine == NULL) return;

    engine->_time_since_last_report += dt;
    if (engine->_time_since_last_report >= 1.0f) {
        engine->_time_since_last_report = clampValue(engine->_time_since_last_report, 1.0f);

        if (engine->_print_report == true) {
            trace_log("UP_TIME: %.2f FPS %.2f MS %.2f", engine->_elapsed_time, engine->_fps, engine->_mpf);
        }
    }
}

static void updateEngine(struct Engine *engine, float dt){
    if (engine == NULL) return;

    updateStats(engine, dt);
    printStats(engine, dt);

    updateQuad(engine->quad[0], dt);
    updateQuad(engine->quad[1], dt);
    updateQuad(engine->quad[2], dt);
    updateCamera(engine->camera, dt);
}

static void renderEngine(struct Engine *engine){
    if (engine == NULL) return;

    renderQuad(engine->quad[0], engine->camera);
    renderQuad(engine->quad[1], engine->camera);
    renderQuad(engine->quad[2], engine->camera);
}

static void initModel(struct Model **modelPtr, struct WfoParser *wfoParser, const char *name) {
    if (modelPtr == NULL || (*modelPtr) != NULL) return;

    unsigned long cubeVboSize = getUnIndexedVertexBufferSizeInFloats(wfoParser, name);
    if (cubeVboSize == 0) return;

    debug_log("Allocating %d * %d = %d for VBO with name %s",
        cubeVboSize, sizeof(float),
        cubeVboSize * sizeof(float),
        name
    );
    float *vbo = calloc(cubeVboSize, sizeof(float));
    if (vbo == NULL) return;

    // TODO: there's a problem here, vertex data isn't being copied properly
    getUnIndexedVertexBuffer(wfoParser, name, vbo, cubeVboSize);
    printf("Dumping vbo contents for %s\n", name);
    for (int i = 0; i < cubeVboSize; ++i) {
        if (i != 0 && i % 8 == 0) {
            printf("\n");
        }
        printf("%.2f ", vbo[i]);
    }
    printf("\nDone with vbo dump\n");

    struct Model *newModel = NULL;
    allocModel(&newModel);
    if (newModel == NULL) {
        free(vbo);
        vbo = NULL;

        return;
    }

    setPNTBuffer(newModel, vbo, cubeVboSize / 8);

    free(vbo);
    vbo = NULL;

    (*modelPtr) = newModel;
    newModel = NULL;
}

void allocEngine(struct Engine **enginePtr){
    if (enginePtr == NULL || (*enginePtr) != NULL) return;

    struct Engine *engine = calloc(1, sizeof(struct Engine));
    if (engine == NULL) return;

    engine->_fps = 0.0f;
    engine->_mpf = 0.0f;
    engine->_elapsed_time = 0.0f;
    engine->_time_since_last_report = 0.0f;
    engine->_print_report = true;

    engine->window = NULL;

    engine->quad[0] = NULL;
    engine->quad[1] = NULL;
    engine->quad[2] = NULL;
    engine->camera = NULL;
    engine->cubeModel = NULL;
    engine->pyramidModel = NULL;
    engine->quadModel = NULL;
    engine->shader = NULL;

    (*enginePtr) = engine;
}

void deleteEngine(struct Engine **enginePtr){
    if (enginePtr == NULL || (*enginePtr) == NULL) return;

    struct Engine *engine = (*enginePtr);
    glfwDestroyWindow(engine->window);
    engine->window = NULL;

    deleteQuad(&(engine->quad[0]));
    deleteQuad(&(engine->quad[1]));
    deleteQuad(&(engine->quad[2]));
    deleteCamera(&(engine->camera));
    deleteModel(&engine->cubeModel);
    deleteModel(&engine->pyramidModel);
    deleteModel(&engine->quadModel);
    deleteShader(&engine->shader);
    engine = NULL;

    glfwTerminate();

    free((*enginePtr));
    (*enginePtr) = NULL;
}

void initEngine(struct Engine *engine){
    if (engine == NULL) return;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        return;
    }

    GLFWwindow *glfwWindow = glfwCreateWindow(800, 600, "Py3DEngine", NULL, NULL);
    if (glfwWindow == NULL) {
        return;
    }
    engine->window = glfwWindow;
    glfwWindow = NULL;

    glfwMakeContextCurrent(engine->window);

    gladLoadGL(glfwGetProcAddress);

    glfwSwapInterval(1);
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    struct WfoParser *wfoParser = NULL;
    allocWfoParser(&wfoParser);

    FILE *wfoFile = fopen("resources/solid_objs.obj", "r");
    parseWaveFrontFile(wfoParser, wfoFile);
    fclose(wfoFile);
    wfoFile = NULL;

    initModel(&engine->cubeModel, wfoParser, "Cube");
    initModel(&engine->pyramidModel, wfoParser, "Pyramid");
    initModel(&engine->quadModel, wfoParser, "Quad");

    deleteWfoParser(&wfoParser);

    allocShader(&engine->shader);
    initShader(engine->shader, vertex_shader_source, fragment_shader_source);

    struct Quad *curQuad = NULL;
    float posW[3] = {0.0f, 0.0f, 2.0f};
    float scale[3] = {1.0f, 1.0f, 1.0f};
    float color[3] = {0.0f, 0.0f, 0.0f};

    posW[0] = -3.0f;
    color[0] = 0.8f;
    color[2] = 0.2f;
    allocQuad(&curQuad, engine->cubeModel, engine->shader);
    setPosWQuad(curQuad, posW);
    setDiffuseColorQuad(curQuad, color);
    engine->quad[0] = curQuad;
    curQuad = NULL;

    posW[0] = 0.0f;
    color[0] = 0.2f;
    color[2] = 0.8f;
    allocQuad(&curQuad, engine->pyramidModel, engine->shader);
    setPosWQuad(curQuad, posW);
    setDiffuseColorQuad(curQuad, color);
    engine->quad[1] = curQuad;
    curQuad = NULL;

    posW[0] = 3.0f;
    color[0] = 0.8f;
    color[1] = 0.8f;
    color[2] = 0.2f;
    scale[0] = 2.0f;
    scale[1] = 2.0f;
    allocQuad(&curQuad, engine->quadModel, engine->shader);
    setPosWQuad(curQuad, posW);
    setScaleQuad(curQuad, scale);
    setDiffuseColorQuad(curQuad, color);
    engine->quad[2] = curQuad;
    curQuad = NULL;

    struct Camera *camera = NULL;
    allocCamera(&camera);
    engine->camera = camera;
    camera = NULL;
}

void runEngine(struct Engine *engine) {
    if (engine == NULL) return;

    float prev_ts, cur_ts = 0.0f;
    while(!glfwWindowShouldClose(engine->window)) {
        prev_ts = cur_ts;
        cur_ts = (float) glfwGetTime();
        float dt = cur_ts - prev_ts;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        updateEngine(engine, dt);
        renderEngine(engine);

        glfwSwapBuffers(engine->window);
        glfwPollEvents();
    }
}
