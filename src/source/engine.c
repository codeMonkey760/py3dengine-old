#include <stdlib.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "logger.h"
#include "config.h"
#include "util.h"
#include "python/python_wrapper.h"
#include "engine.h"
#include "scene_importer.h"
#include "game_object.h"
#include "rendering_context.h"
#include "resource_manager.h"

static float elapsed_time = 0.0f;
static float fps = 0.0f;
static float mpf = 0.0f;
static float time_since_last_report = 0.0f;
static bool print_report = true;

static GLFWwindow *glfwWindow = NULL;

struct ResourceManager *resourceManager = NULL;

struct GameObject *root = NULL;
struct GameObject *activeCamera = NULL;

static void error_callback(int code, const char* description) {
    error_log("%s 0x%x %s\n", "GLFW error code", code, description);
}

static void resizeEngine();

static void resize_window_callback(GLFWwindow *window, int newWidth, int newHeight) {
    resizeEngine();
}

static void updateStats(float dt) {
    static float since_last_calc = 0.0f;
    static int frame_count = 0;

    elapsed_time += dt;
    frame_count++;
    since_last_calc += dt;

    if (print_report == true && since_last_calc >= 1.0f) {
        float _ms = (since_last_calc / (float) frame_count) * 1000.0f;
        float _fps = (float) frame_count;

        frame_count = 0;
        while (since_last_calc >= 1.0f) {
            since_last_calc -= 1.0f;
        }

        fps = _fps;
        mpf = _ms;
    }
}

static void printStats(float dt) {
    time_since_last_report += dt;
    if (time_since_last_report >= 1.0f) {
        time_since_last_report = clampValue(time_since_last_report, 1.0f);

        if (print_report == true) {
            trace_log("UP_TIME: %.2f FPS %.2f MS %.2f", elapsed_time, fps, mpf);
        }
    }
}

static void updateEngine(float dt) {
    updateStats(dt);
    printStats(dt);

    updateGameObject(root, dt);
}

static void renderEngine() {
    struct RenderingContext *renderingContext = NULL;
    allocRenderingContext(&renderingContext);
    initRenderingContext(renderingContext, activeCamera);

    renderGameObject(root, renderingContext);
    deleteRenderingContext(&renderingContext);
}

static void resizeEngine() {
    int newWidth, newHeight;
    glfwGetFramebufferSize(glfwWindow, &newWidth, &newHeight);
    glViewport(0, 0, newWidth, newHeight);
}

void initializeEngine(int argc, char **argv){
    parseConfigFile("config.ini");

    if (!initializePython(argc, argv)) {
        critical_log("%s", "Could not initialize python. Halting");
        return;
    }

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        return;
    }

    GLFWmonitor *primaryMonitor = NULL;
    bool full_screen = getConfigFullScreen();
    if (full_screen == true) {
        primaryMonitor = glfwGetPrimaryMonitor();
    }

    glfwWindow = glfwCreateWindow(getConfigScreenWidth(), getConfigScreenHeight(), "Py3DEngine", primaryMonitor, NULL);
    if (glfwWindow == NULL) {
        return;
    }

    if (full_screen == false) {
        glfwSetWindowPos(glfwWindow, getConfigScreenLeft(), getConfigScreenTop());
    }

    glfwSetWindowSizeCallback(glfwWindow, resize_window_callback);

    glfwMakeContextCurrent(glfwWindow);

    gladLoadGL();

    glfwSwapInterval(getConfigSwapInterval());
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    allocResourceManager(&resourceManager);
    if (resourceManager == NULL) {
        critical_log("%s", "[Engine]: Unable to allocate resource manager. Resource importing will fail");
    }

    struct SceneImporter *importer = NULL;
    allocSceneImporter(&importer);
    if (importer == NULL) {
        critical_log("%s", "[Engine]: Unable to allocate scene importer. Scene parsing will fail");
    }

    const char *startingScenePath = getConfigStartingScene();
    FILE *startingScene = fopen(startingScenePath, "r");
    if (startingScene == NULL) {
        critical_log("[Engine]: Unable to open \"%s\" as scene descriptor for parsing. Cannot initialize", startingScenePath);
    }

    initSceneImporter(importer, resourceManager, &root);
    importScene(importer, startingScene);

    if (startingScene != NULL) {
        fclose(startingScene);
        startingScene = NULL;
    }

    activeCamera = findGameObjectByName(root, "Camera");
    if (activeCamera == NULL) {
        warning_log("%s", "[Engine]: Active Camera could not be set after scene initialization.");
    }

    deleteSceneImporter(&importer);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
}

void runEngine() {
    float prev_ts, cur_ts = 0.0f;
    while(!glfwWindowShouldClose(glfwWindow)) {
        prev_ts = cur_ts;
        cur_ts = (float) glfwGetTime();
        float dt = cur_ts - prev_ts;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        updateEngine(dt);
        renderEngine();

        glfwSwapBuffers(glfwWindow);
        glfwPollEvents();
    }
}

void finalizeEngine() {
    glfwDestroyWindow(glfwWindow);
    deleteResourceManager(&resourceManager);

    deleteGameObject(&root);
    activeCamera = NULL;

    glfwTerminate();

    finalizePython();

    finalizeConfig();
}

void getRenderingTargetDimensions(int *width, int *height) {
    int w = 0, h = 0;

    glfwGetFramebufferSize(glfwWindow, &w, &h);

    if (width != NULL) {
        (*width) = w;
    }
    if (height != NULL) {
        (*height) = h;
    }
}