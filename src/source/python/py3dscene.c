#include "python/py3dscene.h"
#include <structmember.h>

#include "util.h"
#include "engine.h"
#include "logger.h"
#include "python/python_util.h"
#include "physics/collision.h"
#include "python/py3dinput.h"
#include "python/py3dgameobject.h"
#include "python/py3dresourcemanager.h"
#include "python/py3drenderingcontext.h"
#include "lights.h"
#include "python/py3dcomponent.h"
#include "python/py3dlight.h"

static PyObject *py3dSceneCtor = NULL;

struct LightListNode {
    struct Py3dLight *component;
    struct LightListNode *next;
};

static void allocLightListNode(struct LightListNode **node) {
    if (node == NULL || (*node) != NULL) return;

    struct LightListNode *newNode = calloc(1, sizeof(struct LightListNode));
    newNode->component = NULL;
    newNode->next = NULL;

    *node = newNode;
}

static void deallocLightListNode(struct LightListNode **node) {
    if (node == NULL || (*node) == NULL) return;

    deallocLightListNode(&(*node)->next);

    free(*node);
    *node = NULL;
}

static void appendLightToList(struct LightListNode **list, struct Py3dLight *newLightComponent) {
    if (list == NULL) return;

    struct LightListNode *prev = NULL, *next = NULL;

    next = *list;
    while (next != NULL) {
        if (next->component == newLightComponent) {
            warning_log("%s", "[Scene]: Attempted to add non unique light to light list");
            return;
        }

        prev = next;
        next = next->next;
    }

    struct LightListNode *newNode = NULL;
    allocLightListNode(&newNode);
    newNode->next = NULL;
    newNode->component = newLightComponent;

    if (prev == NULL) {
        *list = newNode;
    } else {
        prev->next = newNode;
    }
}

static void removeLightFromList(struct LightListNode **list, const struct Py3dLight *target) {
    if (list == NULL || *list == NULL) {
        warning_log("%s", "[Scene]: Tried to remove light from empty light list");
        return;
    }

    struct LightListNode *prev = NULL, *next = *list;
    while (next != NULL) {
        if (next->component == target) {
            if (prev != NULL) {
                prev->next = next->next;
            } else {
                *list = next->next;
            }

            free(next);
            next = NULL;
            return;
        }

        prev = next;
        next = next->next;
    }

    warning_log("%s", "[Scene]: Attempted to remove a non existent light from light list");
}

static int traverseCallbackTable(struct Py3dScene *self, visitproc visit, void *arg) {
    for (int i = 0; i < GLFW_KEY_MENU+1; ++i) {
        for (int j = 0; j < GLFW_REPEAT+1; ++j) {
            for (int k = 0; k < 64; ++k) {
                Py_VISIT(self->callbackTable[i][j][k]);
            }
        }
    }

    return 0;
}

static int Py3dScene_Traverse(struct Py3dScene *self, visitproc visit, void *arg) {
    Py_VISIT(self->sceneGraph);
    Py_VISIT(self->activeCamera);
    Py_VISIT(self->resourceManager);

    return traverseCallbackTable(self, visit, arg);
}

static void initCallbackTable(struct Py3dScene *self) {
    for (int i = 0; i < GLFW_KEY_MENU+1; ++i) {
        for (int j = 0; j < GLFW_REPEAT+1; ++j) {
            for (int k = 0; k < 64; ++k) {
                self->callbackTable[i][j][k] = NULL;
            }
        }
    }
}

static void finalizeCallbackTable(struct Py3dScene *self) {
    for (int i = 0; i < GLFW_KEY_MENU+1; ++i) {
        for (int j = 0; j < GLFW_REPEAT+1; ++j) {
            for (int k = 0; k < 64; ++k) {
                Py_CLEAR(self->callbackTable[i][j][k]);
            }
        }
    }
}

static int Py3dScene_Clear(struct Py3dScene *self) {
    Py_CLEAR(self->name);
    Py_CLEAR(self->sceneGraph);
    Py_CLEAR(self->activeCamera);
    Py_CLEAR(self->resourceManager);

    return 0;
}

static void Py3dScene_Dealloc(struct Py3dScene *self) {
    trace_log("%s", "[Scene]: Deallocating Scene");

    Py3dScene_Clear(self);
    deallocPhysicsSpace(&self->space);
    finalizeCallbackTable(self);
    LightData_Dealloc(&self->lightData);
    deallocLightListNode(&self->lightList);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dScene_Init(struct Py3dScene *self, PyObject *args, PyObject *kwds) {
    trace_log("%s", "[Scene]: Initializing Scene");

    self->enabled = 1;
    self->visible = 1;
    self->name = Py_NewRef(Py_None);
    self->sceneGraph = Py_NewRef(Py_None);
    self->activeCamera = Py_NewRef(Py_None);
    self->resourceManager = Py_NewRef(Py_None);
    allocPhysicsSpace(&self->space);
    initPhysicsSpace(self->space);
    initCallbackTable(self);
    self->cursorMode = GLFW_CURSOR_NORMAL;
    self->lightData = NULL;
    self->numLights = 0;
    self->lightList = NULL;

    return 0;
}

PyMethodDef Py3dScene_Methods[] = {
    {"enabled", (PyCFunction) Py3dScene_IsEnabled, METH_NOARGS, "Determine if a Scene is enabled"},
    {"enable", (PyCFunction) Py3dScene_Enable, METH_VARARGS, "Enable or disable a Scene"},
    {"visible", (PyCFunction) Py3dScene_IsVisible, METH_NOARGS, "Determine if a Scene is visible"},
    {"make_visible", (PyCFunction) Py3dScene_MakeVisible, METH_VARARGS, "Make a Scene visible or invisible"},
    {"set_key_callback", (PyCFunction) Py3dScene_SetKeyCallback, METH_VARARGS, "Register a callback to be executed when a keyboard event happens"},
    {"set_cursor_mode", (PyCFunction) Py3dScene_SetCursorMode, METH_VARARGS, "Set the cursor mode"},
    {"activate_camera", (PyCFunction) Py3dScene_ActivateCamera, METH_VARARGS, "Activate the supplied camera"},
    {"activate_camera_by_name", (PyCFunction) Py3dScene_ActivateCameraByName, METH_VARARGS, "Find and activate the specified camera"},
    {NULL}
};

PyTypeObject Py3dScene_Type = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "py3dengine.Scene",
    .tp_basicsize = sizeof(struct Py3dScene),
    .tp_dealloc = (destructor) Py3dScene_Dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,
    .tp_doc = "Class for interacting with a scene graph",
    .tp_methods = Py3dScene_Methods,
    .tp_init = (initproc) Py3dScene_Init,
    .tp_new = PyType_GenericNew,
    .tp_traverse = (traverseproc) Py3dScene_Traverse,
    .tp_clear = (inquiry) Py3dScene_Clear
};

int PyInit_Py3dScene(PyObject *module) {
    if (PyType_Ready(&Py3dScene_Type) == -1) return 0;

    if (PyModule_AddObject(module, "Scene", (PyObject *) &Py3dScene_Type) == -1) return 0;

    Py_INCREF(&Py3dScene_Type);

    return 1;
}

int Py3dScene_FindCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "Scene") == 0) {
        critical_log("%s", "[Python]: Py3dScene has not been initialized properly");

        return 0;
    }

    py3dSceneCtor = PyObject_GetAttrString(module, "Scene");

    return 1;
}

void Py3dScene_FinalizeCtor() {
    Py_CLEAR(py3dSceneCtor);
}

int Py3dScene_Check(PyObject *obj) {
    if (obj == NULL) return 0;

    int ret = PyObject_IsInstance(obj, (PyObject *) &Py3dScene_Type);
    if (ret == -1) {
        handleException();
        return 0;
    }

    return ret;
}

struct Py3dScene *Py3dScene_New() {
    if (py3dSceneCtor == NULL) {
        critical_log("%s", "[Python]: Py3dScene has not been initialized properly");

        return NULL;
    }

    PyObject *args = PyTuple_New(0);
    PyObject *py3dScene = PyObject_Call(py3dSceneCtor, args, NULL);
    Py_CLEAR(args);
    if (py3dScene == NULL) {
        critical_log("%s", "[Python]: Failed to create Scene");
        handleException();

        return NULL;
    }

    if (!Py3dScene_Check(py3dScene)) {
        critical_log("%s", "[Python]: Scene ctor did not return scene");

        Py_CLEAR(py3dScene);
    }

    return (struct Py3dScene *) py3dScene;
}

void Py3dScene_Activate(struct Py3dScene *self) {
    if (!Py3dScene_Check((PyObject *) self)) return;

    setCursorMode(self->cursorMode);

    if (!Py3dGameObject_Check(self->sceneGraph)) return;

    PyObject *args = PyTuple_New(0);
    PyObject *ret = Py3dGameObject_Activate((struct Py3dGameObject *) self->sceneGraph, args, NULL);
    Py_CLEAR(args);

    if (ret == NULL) {
        warning_log("[Scene]: Propagating activate message to scene graph raised exception");
        handleException();
    }
    Py_CLEAR(ret);
}

void Py3dScene_Deactivate(struct Py3dScene *self) {
    if (!Py3dScene_Check((PyObject *) self)) return;

    if (!Py3dGameObject_Check(self->sceneGraph)) return;

    PyObject *args = PyTuple_New(0);
    PyObject *ret = Py3dGameObject_Deactivate((struct Py3dGameObject *) self->sceneGraph, args, NULL);
    Py_CLEAR(args);

    if (ret == NULL) {
        warning_log("[Scene]: Propagating deactivate message to scene graph raised exception");
        handleException();
    }
    Py_CLEAR(ret);
}

PyObject *Py3dScene_IsEnabled(struct Py3dScene *self, PyObject *args, PyObject *kwds) {
    return PyBool_FromLong(Py3dScene_IsEnabledBool(self));
}

int Py3dScene_IsEnabledBool(struct Py3dScene *scene) {
    return scene->enabled;
}

PyObject *Py3dScene_Enable(struct Py3dScene *self, PyObject *args, PyObject *kwds) {
    PyObject *enableObj = NULL;
    if (PyArg_ParseTuple(args, "O!", &PyBool_Type, &enableObj) != 1) return NULL;

    Py3dScene_EnableBool(self, Py_IsTrue(enableObj));

    Py_RETURN_NONE;
}

void Py3dScene_EnableBool(struct Py3dScene *scene, int enable) {
    scene->enabled = enable;
}

PyObject *Py3dScene_IsVisible(struct Py3dScene *self, PyObject *args, PyObject *kwds) {
    return PyBool_FromLong(Py3dScene_IsVisibleBool(self));
}

int Py3dScene_IsVisibleBool(struct Py3dScene *scene) {
    return scene->visible;
}

PyObject *Py3dScene_MakeVisible(struct Py3dScene *self, PyObject *args, PyObject *kwds) {
    PyObject *visibleObj = NULL;
    if (PyArg_ParseTuple(args, "O!", &PyBool_Type, &visibleObj) != 1) return NULL;

    Py3dScene_MakeVisibleBool(self, Py_IsTrue(visibleObj));

    Py_RETURN_NONE;
}

void Py3dScene_MakeVisibleBool(struct Py3dScene *scene, int makeVisible) {
    scene->visible = makeVisible;
}

PyObject *Py3dScene_GetName(struct Py3dScene *self, PyObject *Py_UNUSED(ignored)) {
    return Py_NewRef(self->name);
}

void Py3dScene_SetNameCStr(struct Py3dScene *self, const char *newName) {
    if (!Py3dScene_Check((PyObject *) self) || newName == NULL) return;

    PyObject *newNameObj = PyUnicode_FromString(newName);
    if (newNameObj == NULL) {
        handleException();
        return;
    }

    Py_CLEAR(self->name);
    self->name = newNameObj;
}

void Py3dScene_Start(struct Py3dScene *self) {
    if (self == NULL) return;

    if (self->sceneGraph == NULL || !Py3dGameObject_Check(self->sceneGraph)) {
        warning_log("[Scene]: Scene graph is not rooted with a GameObject");
        return;
    }

    PyObject *startArgs = PyTuple_New(0);
    PyObject *startRet = Py3dGameObject_Start((struct Py3dGameObject *) self->sceneGraph, startArgs, NULL);
    if (startRet == NULL) {
        handleException();
    }

    Py_CLEAR(startRet);
    Py_CLEAR(startArgs);
}

void Py3dScene_Update(struct Py3dScene *self, float dt) {
    if (self == NULL) return;

    if (!self->enabled) return;

    if (self->sceneGraph == NULL || !Py3dGameObject_Check(self->sceneGraph)) {
        warning_log("[Scene]: Scene graph is not rooted with a GameObject");
        return;
    }

    PyObject *args = Py_BuildValue("(f)", dt);
    if (args == NULL) {
        handleException();
        return;
    }

    PyObject *ret = Py3dGameObject_Update((struct Py3dGameObject *) self->sceneGraph, args, NULL);
    if (ret == NULL) {
        handleException();
    }

    Py3dScene_RefreshLightingData(self);

    Py_CLEAR(ret);
    Py_CLEAR(args);

    if (self->space == NULL) return;

    handleCollisions(self->space);
}

void Py3dScene_Render(struct Py3dScene *self) {
    if (self == NULL) return;

    if (!self->visible) return;

    if (self->sceneGraph == NULL || !Py3dGameObject_Check(self->sceneGraph)) {
        warning_log("[Scene]: Scene graph is not rooted with a GameObject");
        return;
    }

    if (self->activeCamera == NULL || !Py3dGameObject_Check(self->activeCamera)) {
        warning_log("[Scene]: Scene graph does not possess a valid active camera");
        return;
    }

    struct Py3dRenderingContext *rc = Py3dRenderingContext_New(self);
    if (rc == NULL) {
        handleException();
        return;
    }
    PyObject *args = Py_BuildValue("(O)", rc);

    PyObject *ret = Py3dGameObject_Render((struct Py3dGameObject *) self->sceneGraph, args, NULL);
    if (ret == NULL) {
        handleException();
    }

    Py_CLEAR(ret);
    Py_CLEAR(args);
    Py_CLEAR(rc); // TODO: this might not be correct ... Py3dGameObject_Render should be taking ownership of this???
}

void Py3dScene_End(struct Py3dScene *self) {
    if (self == NULL) return;

    if (self->sceneGraph == NULL || !Py3dGameObject_Check(self->sceneGraph)) {
        warning_log("[Scene]: Scene graph is not rooted with a GameObject");
        return;
    }

    PyObject *endArgs = PyTuple_New(0);
    PyObject *endRet = Py3dGameObject_End((struct Py3dGameObject *) self->sceneGraph, endArgs, NULL);
    if (endRet == NULL) {
        handleException();
    }

    Py_CLEAR(endRet);
    Py_CLEAR(endArgs);
}

void Py3dScene_SetResourceManager(struct Py3dScene *self, PyObject *newManager) {
    if (self == NULL || newManager == NULL) return;

    if (!Py3dResourceManager_Check(newManager)) return;

    Py_CLEAR(self->resourceManager);

    // The caller should be passing its ownership of the resource manager to this scene
    // So the interface for this function "steals" the reference from the caller
    // by NOT incrementing its reference count here with Py_NewRef
    self->resourceManager = newManager;
}

void Py3dScene_SetSceneGraph(struct Py3dScene *self, PyObject *newSceneGraph) {
    if (self == NULL || newSceneGraph == NULL) return;

    if (!Py3dGameObject_Check(newSceneGraph)) return;

    Py_CLEAR(self->activeCamera);
    self->activeCamera = Py_NewRef(Py_None);
    Py_CLEAR(self->sceneGraph);

    // The caller should be passing its ownership of the scene graph to this scene
    // So the interface for this function "steals" the reference from the caller
    // by NOT incrementing its reference count here with Py_NewRef
    self->sceneGraph = newSceneGraph;
}

PyObject *Py3dScene_ActivateCamera(struct Py3dScene *self, PyObject *args, PyObject *kwds) {
    struct Py3dGameObject *newCamera = NULL;

    if (PyArg_ParseTuple(args, "O!", &Py3dGameObject_Type, &newCamera) != 1) return NULL;

    Py_CLEAR(self->activeCamera);
    self->activeCamera = Py_NewRef(newCamera);

    Py_RETURN_NONE;
}

PyObject *Py3dScene_ActivateCameraByName(struct Py3dScene *self, PyObject *args, PyObject *kwds) {
    const char *newCameraName = NULL;

    if (PyArg_ParseTuple(args, "s", &newCameraName) != 1) return NULL;

    return Py3dScene_ActivateCameraByNameCStr(self, newCameraName);
}

PyObject *Py3dScene_ActivateCameraByNameCStr(struct Py3dScene *self, const char *name) {
    if (Py_IsNone(self->sceneGraph)) {
        PyErr_SetString(PyExc_ValueError, "Active camera must exist on scene graph");
        return NULL;
    }

    PyObject *newCamera = Py3dGameObject_GetChildByNameCStr((struct Py3dGameObject *) self->sceneGraph, name);
    if (newCamera == NULL) return NULL;

    if (Py_IsNone(newCamera)) {
        PyErr_SetString(PyExc_ValueError, "Active camera must exist on scene graph");
        Py_CLEAR(newCamera);
        return NULL;
    }

    Py_CLEAR(self->activeCamera);
    // the reference for this comes from Py3dGameObject_GetChildByNameCStr
    // so incrementing the ref count here isn't necessary
    self->activeCamera = newCamera;

    Py_RETURN_NONE;
}

PyObject *Py3dScene_GetActiveCamera(struct Py3dScene *self) {
    if (self->activeCamera == NULL) Py_RETURN_NONE;

    return Py_NewRef(self->activeCamera);
}

// TODO: receive these event from the engine
void Py3dScene_KeyEvent(struct Py3dScene *self, int key, int scancode, int action, int mods) {
    PyObject *callback = self->callbackTable[key][action][mods];
    if (callback == NULL) return;

    PyObject *ret = PyObject_CallNoArgs(callback);
    if (ret == NULL) {
        error_log("[Py3dScene]: Key callback threw exception when handling key:%d action:%d mods:%d", key, action, mods);
        handleException();
    }

    Py_CLEAR(ret);
}

PyObject *Py3dScene_SetKeyCallback(struct Py3dScene *self, PyObject *args, PyObject *kwds) {
    PyObject *callback = NULL, *keyObj = NULL, *actionObj = NULL, *modsObj = NULL;

    PyArg_ParseTuple(args, "OO!O!O!", &callback, &PyLong_Type, &keyObj, &PyLong_Type, &actionObj, &PyLong_Type, &modsObj);
    if (!PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_ValueError, "Param 1 must be callable");
        return NULL;
    }

    int key = convertIntToGlfwKey((int) PyLong_AsLong(keyObj));
    if (key == GLFW_KEY_UNKNOWN) {
        PyErr_SetString(PyExc_ValueError, "Cannot convert param 2 to GLFW Key");
        return NULL;
    }

    int action = (int) PyLong_AsLong(actionObj);
    if (action != GLFW_PRESS && action != GLFW_RELEASE && action != GLFW_REPEAT) {
        PyErr_SetString(PyExc_ValueError, "Cannot convert param 3 to GLFW Action");
        return NULL;
    }

    int mods = (int) PyLong_AsLong(modsObj);
    if (mods > 63) {
        PyErr_SetString(PyExc_ValueError, "Cannot convert param 4 to GLFW Action");
        return NULL;
    }

    Py_INCREF(callback);
    self->callbackTable[key][action][mods] = callback;

    Py_RETURN_NONE;
}

PyObject *Py3dScene_SetCursorMode(struct Py3dScene *self, PyObject *args, PyObject *kwds) {
    char *newMode = NULL;

    if (PyArg_ParseTuple(args, "s", &newMode) != 1) return NULL;

    if (strcmp(newMode, "NORMAL") == 0) {
        self->cursorMode = GLFW_CURSOR_NORMAL;
    } else if (strcmp(newMode, "DISABLED") == 0) {
        self->cursorMode = GLFW_CURSOR_DISABLED;
    } else if (strcmp(newMode, "HIDDEN") == 0) {
        self->cursorMode = GLFW_CURSOR_HIDDEN;
    } else {
        PyErr_SetString(PyExc_ValueError, "Unrecognized cursor mode");
        return NULL;
    }

    setCursorMode(self->cursorMode);

    Py_RETURN_NONE;
}

void Py3dScene_GetDynamicLightData(struct Py3dScene *self, struct LightData **lightDataPtr, size_t *numLightsPtr) {
    if (self == NULL || lightDataPtr == NULL || numLightsPtr == NULL) return;

    (*lightDataPtr) = self->lightData;
    (*numLightsPtr) = self->numLights;
}

void Py3dScene_RefreshLightingData(struct Py3dScene *self) {
    if (self == NULL) return;

    LightData_Dealloc(&self->lightData);
    self->numLights = getConfigMaxDynamicLights();
    LightData_Alloc(&self->lightData, self->numLights);

    memset(self->lightData, 0, sizeof(struct LightData) * self->numLights);

    const struct LightListNode *curNode = self->lightList;
    ssize_t curLight = 0;
    while (curNode != NULL && curLight < self->numLights) {
        struct Py3dLight *component = curNode->component;
        struct Py3dGameObject *owner = Py3d_GetComponentOwner((struct Py3dComponent *) component);
        if (owner == NULL) {
            warning_log("%s", "[Scene]: Could not determine light owner while refreshing lighting data");
            handleException();
            continue;
        }

        self->lightData[curLight].used = 1;
        self->lightData[curLight].enabled =
            Py3dComponent_IsEnabledBool((struct Py3dComponent *) component) &&
            Py3dComponent_IsVisibleBool((struct Py3dComponent *) component) &&
            Py3dGameObject_IsEnabledBool(owner) &&
            Py3dGameObject_IsVisibleBool(owner);
        Py3dLight_GetType(component, &self->lightData[curLight].type);
        Vec3Copy(self->lightData[curLight].position, Py3dGameObject_GetPositionFA(owner));
        Py3dLight_GetDiffuse(component, self->lightData[curLight].diffuse);
        Py3dLight_GetSpecular(component, self->lightData[curLight].specular);
        Py3dLight_GetAmbient(component, self->lightData[curLight].ambient);
        Py3dLight_GetIntensity(component, &self->lightData[curLight].intensity);
        Py3dLight_GetAttenuation(component, self->lightData[curLight].attenuation);

        Py_CLEAR(owner);
        curNode = curNode->next;
        ++curLight;
    }
}

int Py3dScene_RegisterLight(struct Py3dScene *self, struct Py3dLight *newLightComponent) {
    if (self == NULL || newLightComponent == NULL) return 0;

    appendLightToList(&self->lightList, newLightComponent);

    return 1;
}

int Py3dScene_UnRegisterLight(struct Py3dScene *self, const struct Py3dLight *lightComponent) {
    if (self == NULL || lightComponent == NULL) return 0;

    removeLightFromList(&self->lightList, lightComponent);

    return 1;
}