#include <stdlib.h>

#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include "logger.h"
#include "util.h"
#include "python_wrapper.h"
#include "engine.h"
#include "scene_importer.h"
#include "game_object.h"
#include "rendering_context.h"

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

    updateGameObject(engine->root, dt);
}

static void renderEngine(struct Engine *engine){
    if (engine == NULL) return;

    struct RenderingContext *renderingContext = NULL;
    allocRenderingContext(&renderingContext);
    initRenderingContext(renderingContext, engine->activeCamera);

    renderGameObject(engine->root, renderingContext);
    deleteRenderingContext(&renderingContext);
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

    engine->root = NULL;
    engine->activeCamera = NULL;

    (*enginePtr) = engine;
}

void deleteEngine(struct Engine **enginePtr){
    if (enginePtr == NULL || (*enginePtr) == NULL) return;

    struct Engine *engine = (*enginePtr);
    glfwDestroyWindow(engine->window);
    engine->window = NULL;
    deleteResourceManager(&engine->resourceManager);

    deleteGameObject(&engine->root);
    engine->activeCamera = NULL;
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

    initSceneImporter(importer, engine->resourceManager, &engine->root);
    importScene(importer, NULL);
    engine->activeCamera = findGameObjectByName(engine->root, "Camera");
    if (engine->activeCamera == NULL) {
        warning_log("%s", "[Engine]: Active Camera could not be set after scene initialization.");
    }

    deleteSceneImporter(&importer);

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
