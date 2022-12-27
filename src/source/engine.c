#include <stdlib.h>

#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include "logger.h"
#include "util.h"
#include "python_wrapper.h"
#include "shader.h"
#include "engine.h"
#include "scene_importer.h"

static void error_callback(int code, const char* description) {
    error_log("%s 0x%x %s\n", "GLFW error code", code, description);
}

static void updateStats(struct Engine *engine, float dt) {
    // TODO: these static variables won't work for multiple engine objects
    // Not that I foresee that happening ... but the interface implies that as a use case
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

    engine->resourceManager = NULL;

    engine->quad[0] = NULL;
    engine->quad[1] = NULL;
    engine->quad[2] = NULL;
    engine->camera = NULL;

    (*enginePtr) = engine;
}

void deleteEngine(struct Engine **enginePtr){
    if (enginePtr == NULL || (*enginePtr) == NULL) return;

    struct Engine *engine = (*enginePtr);
    glfwDestroyWindow(engine->window);
    engine->window = NULL;
    deleteResourceManager(&engine->resourceManager);

    deleteQuad(&(engine->quad[0]));
    deleteQuad(&(engine->quad[1]));
    deleteQuad(&(engine->quad[2]));
    deleteCamera(&(engine->camera));
    engine = NULL;

    glfwTerminate();

    finalizePython();

    free((*enginePtr));
    (*enginePtr) = NULL;
}

void initEngine(struct Engine *engine){
    if (engine == NULL) return;

    initializePython();

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

    allocResourceManager(&engine->resourceManager);
    if (engine->resourceManager == NULL) {
        critical_log("%s", "[Engine]: Unable to allocate resource manager. Resource importing will fail");
    }

    struct SceneImporter *importer = NULL;
    allocSceneImporter(&importer);
    if (importer == NULL) {
        critical_log("%s", "[Engine]: Unable to allocate scene importer. Scene parsing will fail");
    }

    initSceneImporter(importer, engine->resourceManager);
    importScene(importer, NULL);

    deleteSceneImporter(&importer);

    struct Quad *curQuad = NULL;
    struct Model *curModel = NULL;
    struct Shader *curShader = NULL;
    float posW[3] = {0.0f, 0.0f, 2.0f};
    float color[3] = {0.0f, 0.0f, 0.0f};

    curShader = getShaderResource(engine->resourceManager, "SolidColorShader");
    if (curShader == NULL) {
        critical_log("%s", "Failed to retrieve \"SolidColorShader\" shader");
    }

    posW[0] = -3.0f;
    color[0] = 0.8f;
    color[2] = 0.2f;
    curModel = getModelResource(engine->resourceManager, "Cube");
    if (curModel == NULL) {
        critical_log("%s", "Failed to retrieve \"Cube\" model");
    }
    allocQuad(&curQuad, curModel, curShader);
    setPosWQuad(curQuad, posW);
    setDiffuseColorQuad(curQuad, color);
    engine->quad[0] = curQuad;
    curQuad = NULL;

    posW[0] = 0.0f;
    color[0] = 0.2f;
    color[2] = 0.8f;
    curModel = getModelResource(engine->resourceManager, "Pyramid");
    if (curModel == NULL) {
        critical_log("%s", "Failed to retrieve \"Pyramid\" model");
    }
    allocQuad(&curQuad, curModel, curShader);
    setPosWQuad(curQuad, posW);
    setDiffuseColorQuad(curQuad, color);
    engine->quad[1] = curQuad;
    curQuad = NULL;

    posW[0] = 3.0f;
    color[0] = 0.8f;
    color[1] = 0.8f;
    color[2] = 0.2f;
    curModel = getModelResource(engine->resourceManager, "Quad");
    if (curModel == NULL) {
        critical_log("%s", "Failed to retrieve \"Quad\" model");
    }
    allocQuad(&curQuad, curModel, curShader);
    setPosWQuad(curQuad, posW);
    setDiffuseColorQuad(curQuad, color);
    engine->quad[2] = curQuad;
    curQuad = NULL;

    struct Camera *camera = NULL;
    allocCamera(&camera);
    engine->camera = camera;
    camera = NULL;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
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
