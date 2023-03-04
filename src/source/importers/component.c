#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdlib.h>

#include "logger.h"
#include "python/python_util.h"
#include "python/python_wrapper.h"
#include "python/py3denginemodule.h"
#include "resources/python_script.h"
#include "importers/component.h"

void importComponent(struct PythonScript **scriptPtr, json_object *componentDesc) {
    if (scriptPtr == NULL || (*scriptPtr) != NULL || componentDesc == NULL) return;

    json_object *json_name = json_object_object_get(componentDesc, "name");
    if (json_name == NULL || !json_object_is_type(json_name, json_type_string)) {
        error_log("%s", "[ComponentImporter]: Component descriptor must have a \"name\" field of type string");
        return;
    }
    const char *name = json_object_get_string(json_name);

    json_object *json_import_path = json_object_object_get(componentDesc, "importPath");
    if (json_import_path == NULL || !json_object_is_type(json_import_path, json_type_string)) {
        error_log("%s", "[ComponentImporter]: Component descriptor must have an \"importPath\" field of type string");
        return;
    }
    const char *importPath = json_object_get_string(json_import_path);

    appendImportPath(importPath);

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