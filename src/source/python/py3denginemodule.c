#include "logger.h"
#include "python/python_util.h"
#include "python/py3denginemodule.h"
#include "python/py3dcomponent.h"
#include "python/py3dmodelrenderer.h"
#include "python/py3dspriterenderer.h"
#include "python/py3drenderingcontext.h"
#include "python/py3drigidbody.h"
#include "python/py3dresourcemanager.h"
#include "python/py3dgameobject.h"
#include "python/py3dcontactpoint.h"
#include "python/py3dcollisionevent.h"
#include "python/py3dscene.h"
#include "python/py3dtextrenderer.h"
#include "python/py3dlight.h"
#include "engine.h"

PyObject *Py3dErr_SceneError = NULL;

static PyObject *Py3dEngine_Quit(PyObject *self, PyObject *args, PyObject *kwds) {
    markWindowShouldClose();

    Py_RETURN_NONE;
}

static PyObject *Py3dEngine_LoadScene(PyObject *self, PyObject *args, PyObject *kwds) {
    const char *scenePath = NULL;
    if (PyArg_ParseTuple(args, "s", &scenePath) != 1) return NULL;

    PyObject *ret = (PyObject *) loadScene(scenePath);
    if (ret == NULL) {
        error_log("[Engine]: Could not load scene at path \"%s\"", scenePath);
        return NULL;
    }
    Py_CLEAR(ret);

    Py_RETURN_NONE;
}

static PyObject *Py3dEngine_ActivateScene(PyObject *self, PyObject *args, PyObject *kwds) {
    const char *sceneName = NULL;
    if (PyArg_ParseTuple(args, "s", &sceneName) != 1) return NULL;

    PyObject *ret = activateScene(sceneName);
    if (ret == NULL) {
        error_log("[Engine]: Could not activate scene with name \"%s\"", sceneName);
        return NULL;
    }
    Py_CLEAR(ret);

    Py_RETURN_NONE;
}

static PyObject *Py3dEngine_UnloadScene(PyObject *self, PyObject *args, PyObject *kwds) {
    const char *sceneName = NULL;
    if (PyArg_ParseTuple(args, "s", &sceneName) != 1) return NULL;

    PyObject *ret = unloadScene(sceneName);
    if (ret == NULL) {
        error_log("[Engine]: Could not unload scene with name \"%s\"", sceneName);
        return NULL;
    }
    Py_CLEAR(ret);

    Py_RETURN_NONE;
}

static PyObject *Py3dEngine_GetFPS(PyObject *self, PyObject *args, PyObject *kwds) {
    return PyFloat_FromDouble(getFPS());
}

static PyObject *Py3dEngine_GetMS(PyObject *self, PyObject *args, PyObject *kwds) {
    return PyFloat_FromDouble(getMS());
}

static PyObject *Py3dEngine_GetUptime(PyObject *self, PyObject *args, PyObject *kwds) {
    return PyFloat_FromDouble(getUptime());
}

static PyMethodDef Py3dEngine_Methods[] = {
    {"quit", (PyCFunction) Py3dEngine_Quit, METH_NOARGS, "Stop the engine and begin tear down"},
    {"load_scene", (PyCFunction) Py3dEngine_LoadScene, METH_VARARGS, "Load the specified scene into the engine and prepare it for activation"},
    {"activate_scene", (PyCFunction) Py3dEngine_ActivateScene, METH_VARARGS, "Deactivate the current scene and activate the scene with the specified name"},
    {"unload_scene", (PyCFunction) Py3dEngine_UnloadScene, METH_VARARGS, "Delete the scene with the specified name"},
    {"get_fps", (PyCFunction) Py3dEngine_GetFPS, METH_VARARGS, "Get the \"Frames Per Second\" value from the last time stats were calculated"},
    {"get_ms", (PyCFunction) Py3dEngine_GetMS, METH_VARARGS, "Get the \"Milliseconds Per Frame\" value from the last time stats were calculated"},
    {"get_uptime", (PyCFunction) Py3dEngine_GetUptime, METH_VARARGS, "Get the current engine uptime in seconds"},
    {NULL}
};

static PyModuleDef py3dengineModuleDef = {
    PyModuleDef_HEAD_INIT,
    .m_name = "py3dengineEXT",
    .m_doc = "Top level module containing interface to engine.",
    .m_size = -1,
    .m_methods = Py3dEngine_Methods
};

static PyObject *module = NULL;

PyMODINIT_FUNC
PyInit_py3dEngine(void) {
    PyObject *newModule = PyModule_Create(&py3dengineModuleDef);
    if (newModule == NULL) {
        critical_log("%s", "[Python]: Failed to create \"py3dengine\" module");

        return NULL;
    }

    if (!PyInit_Py3dGameObject(newModule)) {
        critical_log("%s", "[Python]: Failed to attach Game Object to py3dengine module");

        Py_CLEAR(newModule);
        return NULL;
    }

    if (!PyInit_Py3dComponent(newModule)) {
        critical_log("%s", "[Python]: Failed to attach Component to py3dengine module");

        Py_CLEAR(newModule);
        return NULL;
    }

    if (!PyInit_Py3dRenderingContext(newModule)) {
        critical_log("%s", "[Python]: Failed to attach RenderingContext to py3dengine module");

        Py_CLEAR(newModule);
        return NULL;
    }

    if (!PyInit_Py3dResourceManager(newModule)) {
        critical_log("%s", "[Python]: Failed to attach ResourceManager to py3dengine module");

        Py_CLEAR(newModule);
        return NULL;
    }

    if (!PyInit_Py3dModelRenderer(newModule)) {
        critical_log("%s", "[Python]: Failed to attach ModelRendererComponent to py3dengine module");

        Py_CLEAR(newModule);
        return NULL;
    }

    if (!PyInit_Py3dSpriteRenderer(newModule)) {
        critical_log("%s", "[Python]: Failed to attach SpriteRendererComponent to py3dengine module");

        Py_CLEAR(newModule);
        return NULL;
    }

    if (!PyInit_Py3dRigidBody(newModule)) {
        critical_log("%s", "[Python]: Failed to attach ColliderComponent to py3dengine module");

        Py_CLEAR(newModule);
        return NULL;
    }

    if (!PyInit_Py3dContactPoint(newModule)) {
        critical_log("%s", "[Python]: Failed to attach ContactPoint to py3dengine module");

        Py_CLEAR(newModule);
        return NULL;
    }

    if (!PyInit_Py3dCollisionEvent(newModule)) {
        critical_log("%s", "[Python]: Failed to attach CollisionEvent to py3dengine module");

        Py_CLEAR(newModule);
        return NULL;
    }

    if (!PyInit_Py3dScene(newModule)) {
        critical_log("%s", "[Python]: Failed to attach Scene to py3dengine module");

        Py_CLEAR(newModule);
        return NULL;
    }

    if (!PyInit_Py3dTextRenderer(newModule)) {
        critical_log("%s", "[Python]: Failed to attach TextRendererComponent to py3dengine module");

        Py_CLEAR(newModule);
        return NULL;
    }

    if (!PyInit_Py3dLight(newModule)) {
        critical_log("%s", "[Python]: Failed to attach LightComponent to py3dengine module");

        Py_CLEAR(newModule);
        return NULL;
    }

    Py3dErr_SceneError = PyErr_NewException("py3dengineEXT.SceneError", NULL, NULL);
    if (Py3dErr_SceneError == NULL) {
        critical_log("%s", "[Python]: Failed to create SceneError");

        Py_CLEAR(newModule);
        return NULL;
    }

    if (PyModule_AddObject(newModule, "SceneError", Py3dErr_SceneError) == -1) {
        critical_log("%s", "[Python]: Failed to attach SceneActivationException to py3dengine module");

        Py_CLEAR(Py3dErr_SceneError);
        Py_CLEAR(newModule);
        return NULL;
    }

    return newModule;
}

int appendPy3dEngineModule() {
    if (PyImport_AppendInittab("py3dengineEXT", PyInit_py3dEngine) == -1) {
        critical_log("%s", "[Python]: Failed to extend built-in modules table with py3dengineEXT module");
        return false;
    }

    return true;
}

int importPy3dEngineModule() {
    module = PyImport_ImportModule("py3dengineEXT");
    if (module == NULL) {
        critical_log("%s", "[Python]: Could not import py3dengineEXT");
        handleException();
        return false;
    }

    return true;
}

int initPy3dEngineObjects() {
    if (!Py3dGameObject_FindCtor(module)) {
        return false;
    }

    if (!Py3dRenderingContext_FindCtor(module)) {
        return false;
    }

    if (!findPy3dResourceManagerCtor(module)) {
        return false;
    }

    if (!Py3dModelRenderer_FindCtor(module)) {
        return false;
    }

    if (!Py3dSpriteRenderer_FindCtor(module)) {
        return false;
    }

    if (!Py3dContactPoint_FindCtor(module)) {
        return false;
    }

    if (!Py3dCollisionEvent_FindCtor(module)) {
        return false;
    }

    if (!Py3dScene_FindCtor(module)) {
        return false;
    }

    if (!Py3dLight_FindCtor(module)) {
        return false;
    }

    return true;
}

PyObject *getPy3dEngineModule() {
    return module;
}

void finalizePy3dEngineModule() {
    Py_CLEAR(Py3dErr_SceneError);
    Py3dGameObject_FinalizeCtor();
    Py3dRenderingContext_FinalizeCtor();
    finalizePy3dResourceManagerCtor();
    Py3dModelRenderer_FinalizeCtor();
    Py3dSpriteRenderer_FinalizeCtor();
    Py3dContactPoint_FinalizeCtor();
    Py3dCollisionEvent_FinalizeCtor();
    Py3dScene_FinalizeCtor();
    Py3dLight_FinalizeCtor();
}
