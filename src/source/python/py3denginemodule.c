#include "logger.h"
#include "python/python_util.h"
#include "python/py3denginemodule.h"
#include "python/py3dcomponent.h"
#include "python/py3dtransform.h"
#include "rendering_context.h"
#include "resource_manager.h"
#include "game_object.h"

static struct PyModuleDef py3dengineModuleDef = {
    PyModuleDef_HEAD_INIT,
    .m_name = "py3dengine",
    .m_doc = "Top level module containing interface to engine.",
    .m_size = -1,
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
    if (!findPyGameObjectCtor(module)) {
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

    return true;
}

void finalizePy3dEngineModule() {
    Py3dTransform_FinalizeCtor();
    finalizePyGameObjectCtor();
    Py3dRenderingContext_FinalizeCtor();
    finalizePy3dResourceManagerCtor();
}