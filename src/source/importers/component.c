#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdlib.h>

#include "python/python_util.h"
#include "python/py3denginemodule.h"
#include "resources/python_script.h"
#include "importers/component.h"

void importComponent(struct PythonScript **scriptPtr, const char *name) {
    if (scriptPtr == NULL || (*scriptPtr) != NULL) return;

    PyObject *componentModule = PyImport_ImportModule(name);
    if (componentModule == NULL) {
        handleException();
        return;
    }

    struct PythonScript *newScript = NULL;
    allocPythonScript(&newScript);
    if (newScript == NULL) return;

    initPythonScript(newScript, componentModule, name);
    setResourceName((struct BaseResource *) newScript, name);

    (*scriptPtr) = newScript;
    newScript = NULL;
}

void importBuiltinComponent(struct PythonScript **scriptPtr, const char *name) {
    if (scriptPtr == NULL || (*scriptPtr) != NULL) return;

    PyObject *py3dEngineModule = getPy3dEngineModule();
    if (py3dEngineModule == NULL) return;

    struct PythonScript *newScript = NULL;
    allocPythonScript(&newScript);
    if (newScript == NULL) return;

    initPythonScript(newScript, py3dEngineModule, name);
    setResourceName((struct BaseResource *) newScript, name);

    (*scriptPtr) = newScript;
    newScript = NULL;
}