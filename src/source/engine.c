#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <json.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "logger.h"
#include "config.h"
#include "util.h"
#include "python/py3drenderingcontext.h"
#include "python/python_util.h"
#include "python/python_wrapper.h"
#include "engine.h"
#include "importers/scene.h"
#include "python/py3dgameobject.h"
#include "resource_manager.h"
#include "python/py3dinput.h"
#include "physics/collision.h"

static float elapsed_time = 0.0f;
static float fps = 0.0f;
static float mpf = 0.0f;
static float time_since_last_report = 0.0f;
static bool print_report = true;
static GLFWwindow *glfwWindow = NULL;

struct Py3dScene *startingScene = NULL;

static void error_callback(int code, const char* description) {
    error_log("%s 0x%x %s\n", "GLFW error code", code, description);
}

static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // TODO: pass key event to current scene
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

    if (print_report && since_last_calc >= 1.0f) {
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

        if (print_report) {
            trace_log("UP_TIME: %.2f FPS %.2f MS %.2f", elapsed_time, fps, mpf);
        }
    }
}

static void updateEngine(float dt) {
    updateStats(dt);
    printStats(dt);

    if (root == NULL) return;

    PyObject *args = Py_BuildValue("(f)", dt);
    if (args == NULL) {
        handleException();
        return;
    }

    PyObject *ret = Py3dGameObject_Update(root, args, NULL);
    if (ret == NULL) {
        handleException();
    }

    Py_CLEAR(ret);
    Py_CLEAR(args);

    handleCollisions();
}

static void renderEngine() {
    if (root == NULL) return;

    struct Py3dRenderingContext *rc = Py3dRenderingContext_New(activeCamera);
    if (rc == NULL) {
        handleException();
        return;
    }
    PyObject *args = Py_BuildValue("(O)", rc);

    PyObject *ret = Py3dGameObject_Render(root, args, NULL);
    if (ret == NULL) {
        handleException();
    }

    Py_CLEAR(ret);
    Py_CLEAR(args);
    Py_CLEAR(rc);
}

static void startEngine() {
    PyObject *startCallable = PyObject_GetAttrString((PyObject *) root, "start");
    if (startCallable == NULL || PyCallable_Check(startCallable) == 0) {
        critical_log("[Engine]: Root GameObject must have callable attribute called \"start\"");
        Py_CLEAR(startCallable);
        handleException();
        return;
    }

    PyObject *startArgs = PyTuple_New(0);
    PyObject *startRet = PyObject_Call(startCallable, startArgs, NULL);
    if (startRet == NULL) {
        handleException();
    }

    Py_CLEAR(startRet);
    Py_CLEAR(startCallable);
    Py_CLEAR(startArgs);
}

static void resizeEngine() {
    int newWidth, newHeight;
    glfwGetFramebufferSize(glfwWindow, &newWidth, &newHeight);
    glViewport(0, 0, newWidth, newHeight);
}

static void endEngine() {
    PyObject *endCallable = PyObject_GetAttrString((PyObject *) root, "end");
    if (endCallable == NULL || PyCallable_Check(endCallable) == 0) {
        critical_log("[Engine]: Root GameObject must have callable attribute called \"end\"");
        Py_CLEAR(endCallable);
        handleException();
        return;
    }

    PyObject *endArgs = PyTuple_New(0);
    PyObject *endRet = PyObject_Call(endCallable, endArgs, NULL);
    if (endRet == NULL) {
        handleException();
    }

    Py_CLEAR(endRet);
    Py_CLEAR(endCallable);
    Py_CLEAR(endArgs);
}

void initializeEngine(int argc, char **argv){
    parseConfigFile("config.json");

    if (!initializePython(argc, argv)) {
        critical_log("%s", "Could not initialize python. Halting");
        return;
    }

    if (!dInitODE2(0)) {
        critical_log("%s", "[Engine]: Could not initialize collision engine");
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
    glfwSetKeyCallback(glfwWindow, glfw_key_callback);

    glfwMakeContextCurrent(glfwWindow);

    gladLoadGL(glfwGetProcAddress);

    glfwSwapInterval(getConfigSwapInterval());
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    const char *startingScenePath = getConfigStartingScene();
    FILE *startingSceneFile = fopen(startingScenePath, "r");
    if (startingSceneFile == NULL) {
        critical_log("[Engine]: Unable to open \"%s\" as scene descriptor for parsing. Cannot initialize", startingScenePath);
        return;
    }

    json_object *startingSceneJson = json_object_from_fd(fileno(startingSceneFile));
    if (startingSceneJson == NULL) {
        critical_log("%s", "[Engine]: Could not parse scene descriptor");
        return;
    }

    startingScene = importScene(startingSceneJson);

    json_object_put(startingSceneJson);

    if (startingScene == NULL) {
        critical_log("%s", "[Engine]: Scene parser raised exception while parsing");
        handleException();
        return;
    }

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    startEngine();
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
    endEngine();

    glfwDestroyWindow(glfwWindow);

    Py_CLEAR(startingScene);
    forceGarbageCollection();

    trace_log("[Engine]: Post scene de-allocation python object dump");
    dumpPythonObjects();

    glfwTerminate();

    finalizePython();

    dCloseODE();

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

void markWindowShouldClose() {
    glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
}

int getCursorMode() {
    return glfwGetInputMode(glfwWindow, GLFW_CURSOR);
}

void setCursorMode(int newMode) {
    glfwSetInputMode(glfwWindow, GLFW_CURSOR, newMode);
}