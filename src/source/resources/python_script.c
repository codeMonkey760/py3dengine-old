#include "resources/python_script.h"
#include "custom_string.h"
#include "logger.h"
#include "components/python_component.h"

#define RESOURCE_TYPE_PYTHON_SCRIPT 5

static void delete(struct BaseResource **componentPtr) {
    if (componentPtr == NULL) return;

    if (!isResourceTypePythonScript((*componentPtr))) return;

    deletePythonScript((struct PythonScript **) componentPtr);
}

bool isResourceTypePythonScript(struct BaseResource *resource){
    if (resource == NULL) return false;

    return stringEqualsCStr(resource->_typeName, RESOURCE_TYPE_NAME_PYTHON_SCRIPT);
}

void allocPythonScript(struct PythonScript **pythonScriptPtr){
    if (pythonScriptPtr == NULL || (*pythonScriptPtr) != NULL) return;

    struct PythonScript *newResource = calloc(1, sizeof(struct PythonScript));
    if (newResource == NULL) return;

    struct BaseResource *base = (struct BaseResource *) newResource;
    base->_type = RESOURCE_TYPE_PYTHON_SCRIPT;
    allocString(&base->_typeName, RESOURCE_TYPE_NAME_PYTHON_SCRIPT);
    base->delete = delete;
    base = NULL;

    newResource->_componentType = NULL;

    (*pythonScriptPtr) = newResource;
    newResource = NULL;
}

void deletePythonScript(struct PythonScript **pythonScriptPtr){
    if (pythonScriptPtr == NULL || (*pythonScriptPtr) == NULL) return;

    struct PythonScript *resource = (*pythonScriptPtr);
    Py_CLEAR(resource->_componentType);

    finalizeBaseResource((struct BaseResource *) (*pythonScriptPtr));

    free((*pythonScriptPtr));
    (*pythonScriptPtr) = NULL;
    resource = NULL;
}

void initPythonScript(struct PythonScript *resource, PyObject *module, const char *newComponentTypeName){
    if (resource == NULL || module == NULL || newComponentTypeName == NULL) return;

    PyObject *componentType = PyObject_GetAttrString(module, newComponentTypeName);
    if (componentType == NULL) {
        error_log("[PythonScript]: Could not find component type \"%s\"", newComponentTypeName);

        return;
    }

    if (PyCallable_Check(componentType) < 0) {
        error_log("[PythonScript]: Component type for \"%s\" must be callable", newComponentTypeName);

        return;
    }

    Py_INCREF(componentType);
    Py_CLEAR(resource->_componentType);
    resource->_componentType = componentType;
}

void createPythonComponent(struct PythonScript *resource, struct PythonComponent **componentPtr) {
    if (resource == NULL || resource->_componentType == NULL || componentPtr == NULL || (*componentPtr) != NULL) return;

    struct PythonComponent *newComponent = NULL;
    allocPythonComponent(&newComponent);
    if (newComponent == NULL) return;

    initPythonComponent(newComponent, resource->_componentType);

    (*componentPtr) = newComponent;
    newComponent = NULL;
}

PyObject *getPythonScriptType(struct PythonScript *resource) {
    if (resource == NULL) return NULL;

    return resource->_componentType;
}