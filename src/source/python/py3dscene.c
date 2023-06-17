#include "python/py3dscene.h"
#include <structmember.h>

#include "logger.h"
#include "python/python_util.h"
#include "physics/collision.h"
#include "python/py3dinput.h"

static PyObject *py3dSceneCtor = NULL;

static int Py3dScene_Traverse(struct Py3dScene *self, visitproc visit, void *arg) {
    Py_VISIT(self->sceneGraph);
    Py_VISIT(self->activeCamera);

    return 0;
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
    Py_CLEAR(self->sceneGraph);
    Py_CLEAR(self->activeCamera);
    deallocPhysicsSpace(&self->space);
    finalizeCallbackTable(self);

    return 0;
}

static void Py3dScene_Dealloc(struct Py3dScene *self) {
    trace_log("%s", "[Scene]: Deallocating Scene");

    Py3dScene_Clear(self);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dScene_Init(struct Py3dScene *self, PyObject *args, PyObject *kwds) {
    trace_log("%s", "[Scene]: Initializing Scene");

    self->enabled = 1;
    self->visible = 1;
    self->sceneGraph = Py_NewRef(Py_None);
    self->activeCamera = Py_NewRef(Py_None);
    allocPhysicsSpace(&self->space);
    initPhysicsSpace(self->space);

    return 0;
}

PyMethodDef Py3dScene_Methods[] = {
    {"enabled", (PyCFunction) Py3dScene_IsEnabled, METH_NOARGS, "Determine if a Scene is enabled"},
    {"enable", (PyCFunction) Py3dScene_Enable, METH_VARARGS, "Enable or disable a Scene"},
    {"visible", (PyCFunction) Py3dScene_IsVisible, METH_NOARGS, "Determine if a Scene is visible"},
    {"make_visible", (PyCFunction) Py3dScene_MakeVisible, METH_VARARGS, "Make a Scene visible or invisible"},
    {"set_key_callback", (PyCFunction) Py3dScene_SetKeyCallback, METH_VARARGS, "Register a callback to be executed when a keyboard event happens"},
    {"set_cursor_mode", (PyCFunction) Py3dInput_SetCursorMode, METH_VARARGS, "Set the cursor mode"},
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
    PyObject *py3dScene = PyObject_Call(py3dScene, args, NULL);
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
    // TODO: call the engine and set this scene's cursor mode

    // TODO: propagate the activation message to the scene graph
}

void P73dScene_Deactivate(struct Py3dScene *self) {
    //TODO: propagate the deactivation message to the scene graph
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

PyObject *Py3dInput_SetCursorMode(struct Py3dScene *self, PyObject *args, PyObject *kwds) {
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

    // TODO: use the engine to set this
    glfwSetInputMode(glfwWindow, GLFW_CURSOR, self->cursorMode);

    Py_RETURN_NONE;
}