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
#include "physics/collision.h"
#include "python/py3dscene.h"

extern PyObject *Py3dErr_SceneError;

static float elapsed_time = 0.0f;
static float fps = 0.0f;
static float mpf = 0.0f;
static float time_since_last_report = 0.0f;
static bool print_report = true;
static PyObject *sceneList = NULL;
static struct Py3dScene *activeScene = NULL;
static struct Py3dScene *sceneAwaitingActivation = NULL;

GLFWwindow *glfwWindow = NULL;

static void error_callback(int code, const char* description) {
    error_log("%s 0x%x %s\n", "GLFW error code", code, description);
}

static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (activeScene == NULL) return;

    Py3dScene_KeyEvent(activeScene, key, scancode, action, mods);
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

static void resizeEngine() {
    int newWidth, newHeight;
    glfwGetFramebufferSize(glfwWindow, &newWidth, &newHeight);
    glViewport(0, 0, newWidth, newHeight);
}

static void doSceneActivation() {
    if (sceneAwaitingActivation == NULL) return;

    if (activeScene != NULL) {
        trace_log("[Engine]: Deactivating scene");
        Py3dScene_Deactivate(activeScene);
        Py_CLEAR(activeScene);
    }

    activeScene = sceneAwaitingActivation;
    sceneAwaitingActivation = NULL;

    trace_log("[Engine]: Activating scene");
    Py3dScene_Activate(activeScene);
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
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    sceneList = PyList_New(0);
    PyObject *ret = loadScene(getConfigStartingScene());
    if (ret == NULL) {
        critical_log("[Engine]: Could not load starting scene");
        handleException();
    }
    Py_CLEAR(ret);

    if (PyList_Size(sceneList) < 1) {
        critical_log("[Engine]: Could not schedule starting scene for activation");
    } else {
        sceneAwaitingActivation = (struct Py3dScene *) Py_NewRef(PyList_GetItem(sceneList, 0));
    }
}

void runEngine() {
    float prev_ts, cur_ts = 0.0f;
    while(!glfwWindowShouldClose(glfwWindow)) {
        prev_ts = cur_ts;
        cur_ts = (float) glfwGetTime();
        float dt = cur_ts - prev_ts;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        updateStats(dt);
        printStats(dt);

        doSceneActivation();

        Py3dScene_Update(activeScene, dt);
        Py3dScene_Render(activeScene);

        glfwSwapBuffers(glfwWindow);
        glfwPollEvents();
    }
}

static void endLoadedScenes() {
    Py_ssize_t sceneListLen = PyList_Size(sceneList);
    for (Py_ssize_t i = 0; i < sceneListLen; ++i) {
        PyObject *curScene = PyList_GetItem(sceneList, i);
        if (!Py3dScene_Check(curScene)) {
            critical_log("[Engine]: Non scene object or null detected in scene list");
            continue;
        }

        PyObject *curSceneNameObj = Py3dScene_GetName((struct Py3dScene *) curScene, NULL);
        trace_log("[Engine]: Ending scene with name \"%s\"", PyUnicode_AsUTF8(curSceneNameObj));
        Py_CLEAR(curSceneNameObj);

        Py3dScene_End((struct Py3dScene *) curScene);
    }
}

void finalizeEngine() {
    trace_log("[Engine]: Deactivating scene");
    Py3dScene_Deactivate(activeScene);

    endLoadedScenes();

    glfwDestroyWindow(glfwWindow);

    Py_CLEAR(activeScene);
    Py_CLEAR(sceneList);
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

PyObject *loadScene(const char *scenePath) {
    FILE *sceneFile = fopen(scenePath, "r");
    if (sceneFile == NULL) {
        PyErr_Format(PyExc_ValueError, "Unable to open \"%s\" as scene descriptor", scenePath);
        return NULL;
    }

    json_object *sceneJson = json_object_from_fd(fileno(sceneFile));
    if (sceneJson == NULL) {
        PyErr_Format(PyExc_ValueError, "Could not parse \"%s\" as JSON", scenePath);
        return NULL;
    }

    trace_log("[Engine]: Loading scene at \"%s\"", scenePath);

    struct Py3dScene *scene = importScene(sceneJson);
    json_object_put(sceneJson);
    fclose(sceneFile);
    sceneFile = NULL;
    if (!Py3dScene_Check((PyObject *) scene)) {
        error_log("[Engine]: Parsing scene at \"%s\" failed", scenePath);
        Py_CLEAR(scene);
        return NULL;
    }
    if (PyList_Append(sceneList, (PyObject *) scene) != 0) {
        critical_log("[Engine]: Could not append scene to scene list");
        Py_CLEAR(scene);
        return NULL;
    }

    trace_log("[Engine]: Starting scene loaded from \"%s\"", scenePath);
    Py3dScene_Start(scene);
    Py_CLEAR(scene);

    Py_RETURN_NONE;
}

PyObject *activateScene(const char *sceneName) {
    if (sceneName == NULL) {
        PyErr_SetString(PyExc_ValueError, "Cannot activate scene without name");
        return NULL;
    }

    if (sceneAwaitingActivation != NULL) {
        PyErr_SetString(Py3dErr_SceneError, "Scene activation is unavailable while another scene is awaiting activation");
        return NULL;
    }

    PyObject *sceneNameObj = PyUnicode_FromString(sceneName);
    if (sceneName == NULL) return NULL;

    Py_ssize_t sceneListLen = PyList_Size(sceneList);
    for (Py_ssize_t i = 0; i < sceneListLen; ++i) {
        PyObject *curScene = PyList_GetItem(sceneList, i);
        if (!Py3dScene_Check(curScene)) {
            critical_log("[Engine]: Non scene object or null detected in scene list");
            continue;
        }

        PyObject *curSceneNameObj = Py3dScene_GetName((struct Py3dScene *) curScene, NULL);
        int cmpRet = PyObject_RichCompareBool(sceneNameObj, curSceneNameObj, Py_EQ);
        Py_CLEAR(curSceneNameObj);

        // curScene was a borrowed ref, do not clear it
        if (cmpRet == -1) {
            handleException();
            continue;
        } else if (cmpRet == 0) {
            continue;
        } else {
            trace_log("[Engine]: Scheduling scene named \"%s\" for activation", sceneName);
            sceneAwaitingActivation = (struct Py3dScene *) Py_NewRef(curScene);
            Py_CLEAR(sceneNameObj);
            Py_RETURN_NONE;
        }
    }

    Py_CLEAR(sceneNameObj);
    PyErr_Format(Py3dErr_SceneError, "Could not activate scene with name \"%s\". Please load it first.", sceneName);
    return NULL;
}

PyObject *unloadScene(const char *sceneName) {
    if (sceneName == NULL) {
        PyErr_SetString(PyExc_ValueError, "Cannot unload scene without name");
        return NULL;
    }

    if (sceneAwaitingActivation != NULL) {
        PyErr_SetString(Py3dErr_SceneError, "Scene unloading is unavailable while another scene is awaiting activation");
        return NULL;
    }

    PyObject *sceneNameObj = PyUnicode_FromString(sceneName);
    if (sceneName == NULL) return NULL;

    PyObject *activeSceneName = Py3dScene_GetName(activeScene, NULL);
    int cmpRet = PyObject_RichCompareBool(sceneNameObj, activeSceneName, Py_EQ);
    Py_CLEAR(activeSceneName);
    if (cmpRet == -1) {
        critical_log("[Engine]: Could not compare unloading scene name with active scene name");
        Py_CLEAR(sceneNameObj);
        return NULL;
    } else if (cmpRet == 1) {
        PyErr_SetString(Py3dErr_SceneError, "Cannot unload the currently active scene");
        Py_CLEAR(sceneNameObj);
        return NULL;
    }

    Py_ssize_t sceneListLen = PyList_Size(sceneList);
    Py_ssize_t targetIndex = -1;
    for (Py_ssize_t i = 0; i < sceneListLen; ++i) {
        PyObject *curScene = PyList_GetItem(sceneList, i);
        if (!Py3dScene_Check(curScene)) {
            critical_log("[Engine]: Non scene object or null detected in scene list");
            continue;
        }

        PyObject *curSceneNameObj = Py3dScene_GetName((struct Py3dScene *) curScene, NULL);
        cmpRet = PyObject_RichCompareBool(sceneNameObj, curSceneNameObj, Py_EQ);
        Py_CLEAR(curSceneNameObj);

        // curScene was a borrowed ref, do not clear it
        if (cmpRet == -1) {
            handleException();
            continue;
        } else if (cmpRet == 0) {
            continue;
        } else {
            targetIndex = i;
            break;
        }
    }
    Py_CLEAR(sceneNameObj);

    if (targetIndex == -1) {
        PyErr_Format(Py3dErr_SceneError, "Could not find scene with name \"%\" for unloading", sceneName);
        return NULL;
    }

    Py3dScene_End((struct Py3dScene *) PyList_GetItem(sceneList, targetIndex));
    PySequence_DelItem(sceneList, targetIndex);
    forceGarbageCollection();
    Py_RETURN_NONE;
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