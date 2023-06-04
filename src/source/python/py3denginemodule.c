#include "logger.h"
#include "python/python_util.h"
#include "python/py3denginemodule.h"
#include "python/py3dcomponent.h"
#include "python/py3dtransform.h"
#include "python/py3dmodelrenderer.h"
#include "python/py3dspriterenderer.h"
#include "python/py3drenderingcontext.h"
#include "python/py3dcollider.h"
#include "resource_manager.h"
#include "python/py3dgameobject.h"
#include "python/py3dcontactpoint.h"
#include "python/py3dcollisionevent.h"
#include "python/py3dscene.h"
#include "engine.h"

static PyObject *Py3dEngine_Quit(PyObject *self, PyObject *args, PyObject *kwds) {
    markWindowShouldClose();

    Py_RETURN_NONE;
}

static PyMethodDef Py3dEngine_Methods[] = {
    {"quit", (PyCFunction) Py3dEngine_Quit, METH_NOARGS, "Stop the engine and begin tear down"},
    {NULL}
};

static struct PyModuleDef py3dengineModuleDef = {
    PyModuleDef_HEAD_INIT,
    .m_name = "py3dengine",
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

    if (!PyInit_Py3dTransform(newModule)) {
        critical_log("%s", "[Python]: Failed to attach Transform to py3dengine module");

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

    if (!PyInit_Py3dCollider(newModule)) {
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

    return newModule;
}

bool appendPy3dEngineModule() {
    if (PyImport_AppendInittab("py3dengine", PyInit_py3dEngine) == -1) {
        critical_log("%s", "[Python]: Failed to extend built-in modules table with py3dengine module");
        return false;
    }

    return true;
}

bool importPy3dEngineModule() {
    module = PyImport_ImportModule("py3dengine");
    if (module == NULL) {
        critical_log("%s, [Python]: Could not import py3dengine");
        handleException();
        return false;
    }

    return true;
}

bool initPy3dEngineObjects() {
    if (!Py3dGameObject_FindCtor(module)) {
        return false;
    }

    if (!Py3dTransform_FindCtor(module)) {
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

    if (!Py3dCollider_FindCtor(module)) {
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

    return true;
}

PyObject *getPy3dEngineModule() {
    return module;
}

void finalizePy3dEngineModule() {
    Py3dTransform_FinalizeCtor();
    Py3dGameObject_FinalizeCtor();
    Py3dRenderingContext_FinalizeCtor();
    finalizePy3dResourceManagerCtor();
    Py3dModelRenderer_FinalizeCtor();
    Py3dSpriteRenderer_FinalizeCtor();
    Py3dCollider_FinalizeCtor();
    Py3dContactPoint_FinalizeCtor();
    Py3dCollisionEvent_FinalizeCtor();
    Py3dScene_FinalizeCtor();
}