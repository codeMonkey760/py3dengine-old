#include <json.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "logger.h"
#include "config.h"
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
static PyObject *sceneDict = NULL;
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

    if (since_last_calc >= 1.0f) {
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

    sceneDict = PyDict_New();
    struct Py3dScene *ret = loadScene(getConfigStartingScene());
    if (ret == NULL) {
        critical_log("[Engine]: Could not load starting scene");
        handleException();
    } else {
        sceneAwaitingActivation = ret;
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

        doSceneActivation();

        Py3dScene_Update(activeScene, dt);
        Py3dScene_Render(activeScene);

        glfwSwapBuffers(glfwWindow);
        glfwPollEvents();
    }
}

static void endLoadedScenes() {
    PyObject *sceneDictValues = PyDict_Values(sceneDict);

    Py_ssize_t sceneDictLen = PyList_Size(sceneDictValues);
    for (Py_ssize_t i = 0; i < sceneDictLen; ++i) {
        PyObject *curScene = PyList_GetItem(sceneDictValues, i);
        if (!Py3dScene_Check(curScene)) {
            critical_log("[Engine]: Non scene object or null detected in scene list");
            continue;
        }

        PyObject *curSceneNameObj = Py3dScene_GetName((struct Py3dScene *) curScene, NULL);
        trace_log("[Engine]: Ending scene with name \"%s\"", PyUnicode_AsUTF8(curSceneNameObj));
        Py_CLEAR(curSceneNameObj);

        Py3dScene_End((struct Py3dScene *) curScene);
    }

    Py_CLEAR(sceneDictValues);
}

void finalizeEngine() {
    trace_log("[Engine]: Deactivating scene");
    Py3dScene_Deactivate(activeScene);

    endLoadedScenes();

    glfwDestroyWindow(glfwWindow);

    Py_CLEAR(activeScene);
    Py_CLEAR(sceneDict);
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

struct Py3dScene *loadScene(const char *scenePath) {
    FILE *sceneFile = fopen(scenePath, "r");
    if (sceneFile == NULL) {
        PyErr_Format(PyExc_ValueError, "Unable to open \"%s\" as scene descriptor", scenePath);
        return NULL;
    }

    json_object *sceneJson = json_object_from_fd(fileno(sceneFile));
    if (sceneJson == NULL) {
        PyErr_Format(PyExc_ValueError, "Could not parse \"%s\" as JSON", scenePath);
        fclose(sceneFile);
        return NULL;
    }

    const char *sceneName = peekSceneName(sceneJson);
    if (sceneName == NULL) {
        PyErr_SetString(PyExc_ValueError, "Cannot load scene without name property");
        json_object_put(sceneJson);
        fclose(sceneFile);
        return NULL;
    }

    PyObject *sceneNameObj = PyUnicode_FromString(sceneName);
    if (sceneNameObj == NULL) {
        PyErr_SetString(PyExc_ValueError, "Cannot load scene without name property");
        json_object_put(sceneJson);
        fclose(sceneFile);
        return NULL;
    }

    int sceneDictHasName = PyDict_Contains(sceneDict, sceneNameObj);
    if (sceneDictHasName == -1) {
        Py_CLEAR(sceneNameObj);
        json_object_put(sceneJson);
        fclose(sceneFile);
        return NULL;
    } else if (sceneDictHasName == 1) {
        PyErr_Format(Py3dErr_SceneError, "Scene with name \"%s\" is already loaded", sceneName);
        Py_CLEAR(sceneNameObj);
        json_object_put(sceneJson);
        fclose(sceneFile);
        return NULL;
    }

    trace_log("[Engine]: Loading scene with name \"%s\"", sceneName);

    struct Py3dScene *scene = importScene(sceneJson);

    if (!Py3dScene_Check((PyObject *) scene)) {
        error_log("[Engine]: Parsing scene at \"%s\" failed", scenePath);
        Py_CLEAR(sceneNameObj);
        Py_CLEAR(scene);
        json_object_put(sceneJson);
        fclose(sceneFile);
        return NULL;
    }
    if (PyDict_SetItem(sceneDict, sceneNameObj, (PyObject *) scene) != 0) {
        critical_log("[Engine]: Could not store scene");
        Py_CLEAR(sceneNameObj);
        Py_CLEAR(scene);
        json_object_put(sceneJson);
        fclose(sceneFile);
        return NULL;
    }
    Py_CLEAR(sceneNameObj);

    trace_log("[Engine]: Starting scene with name \"%s\"", sceneName);
    Py3dScene_Start(scene);

    json_object_put(sceneJson);
    fclose(sceneFile);
    sceneFile = NULL;

    return scene;
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

    PyObject *scene = PyDict_GetItemString(sceneDict, sceneName);
    if (!Py3dScene_Check(scene)) {
        PyErr_Format(Py3dErr_SceneError, "Could not activate scene with name \"%s\". Please load it first.", sceneName);
        Py_CLEAR(scene);
        return NULL;
    }

    trace_log("[Engine]: Scheduling scene named \"%s\" for activation", sceneName);
    sceneAwaitingActivation = (struct Py3dScene *) Py_NewRef(scene);
    Py_RETURN_NONE;
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

    PyObject *target = PyDict_GetItem(sceneDict, sceneNameObj);
    if (target == NULL) {
        PyErr_Format(Py3dErr_SceneError, "Could not find scene with name \"%\" for unloading", sceneName);
        Py_CLEAR(sceneNameObj);
        return NULL;
    }

    if (Py3dScene_Check(target)) {
        Py3dScene_End((struct Py3dScene *) target);
    } else {
        warning_log("[Engine]: A non scene object was found in the scene dict under the key \"%s\"", sceneName);
    }

    if (PyDict_DelItem(sceneDict, sceneNameObj) != 0) {
        warning_log("[Engine]: Unable to delete object with key \"%s\" from scene dict", sceneName);
        handleException();
    }
    Py_CLEAR(sceneNameObj);

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

float getFPS() {
    return fps;
}

float getMS() {
    return mpf;
}

float getUptime() {
    return elapsed_time;
}
