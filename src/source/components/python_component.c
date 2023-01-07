#include <stdlib.h>
#include <stdbool.h>

#include "components/python_component.h"
#include "rendering_context.h"
#include "custom_string.h"
#include "logger.h"
#include "python/python_util.h"

#define COMPONENT_TYPE_PYTHON 5

static bool isComponentValid(struct BaseComponent *component) {
    if (component == NULL) return false;

    return component->_type == COMPONENT_TYPE_PYTHON;
}

static void callPythonFunction(PyObject *callable, PyObject *args) {
    if (callable == NULL || args == NULL) return;

    if (PyCallable_Check(callable) != 1) return;

    PyObject *ret = PyObject_Call(callable, args, NULL);
    if (ret == NULL) {
        handleException();
    } else if (Py_IsNone(ret) == 0) {
        debug_log(
            "[PythonComponent]: Component function returned \"%s\" instead of None. Which is weird.",
            Py_TYPE(ret)->tp_name //TODO: make sure this is sound / future proof ... looks ugly
        );
    }
}

static void update(struct BaseComponent *component, float dt) {
    if (!isComponentValid(component)) return;

    struct PythonComponent *pythonComponent = (struct PythonComponent *) component;
    if (pythonComponent->pyComponent == NULL) return;

    if (PyObject_HasAttrString(pythonComponent->pyComponent, "update") == 0) return;

    callPythonFunction(
        PyObject_GetAttrString(pythonComponent->pyComponent, "update"),
        Py_BuildValue("(f)", dt)
    );
}

static void render(struct BaseComponent *component, struct RenderingContext *renderingContext) {
    if  (!isComponentValid(component)) return;

    struct PythonComponent *pythonComponent = (struct PythonComponent *) component;
    if (pythonComponent->pyComponent == NULL) return;

    if (PyObject_HasAttrString(pythonComponent->pyComponent, "render") == 0) return;

    // TODO: eventually pythonize rendering context and pass that instead of empty tuple
    callPythonFunction(
        PyObject_GetAttrString(pythonComponent->pyComponent, "render"),
        PyTuple_New(0) // TODO: docs say to pass an empty tuple if no args ... is this right?
    );
}

static void delete(struct BaseComponent **componentPtr) {
    if (componentPtr == NULL) return;

    if (!isComponentValid( (*componentPtr) )) return;

    deletePythonComponent((struct PythonComponent **) componentPtr);
}

void allocPythonComponent(struct PythonComponent **componentPtr) {
    if (componentPtr == NULL || (*componentPtr) != NULL) return;

    struct PythonComponent *newComponent = calloc(1, sizeof(struct PythonComponent));
    if (newComponent == NULL) return;

    struct BaseComponent *base = (struct BaseComponent *) newComponent;
    initializeBaseComponent(base);
    base->_type = COMPONENT_TYPE_PYTHON;
    allocString(&base->_typeName, COMPONENT_TYPE_NAME_PYTHON);
    base->update = update;
    base->render= render;
    base->delete = delete;
    base = NULL;

    (*componentPtr) = newComponent;
    newComponent = NULL;
}

void deletePythonComponent(struct PythonComponent **componentPtr) {
    if (componentPtr == NULL || (*componentPtr) == NULL) return;

    struct PythonComponent *component = (*componentPtr);
    Py_CLEAR(component->pyComponent);
    component = NULL;

    finalizeBaseComponent((struct BaseComponent *) (*componentPtr));

    free( (*componentPtr) );
    (*componentPtr) = NULL;
}

void initPythonComponent(struct PythonComponent *component, PyObject *componentType) {
    if (component == NULL || componentType == NULL) return;

    if (PyCallable_Check(componentType) != 1) {
        error_log("%s", "[PythonComponent]: Component type must be callable");

        return;
    }

    PyObject *newPyComponent = PyObject_Call(componentType, PyTuple_New(0), NULL);
    if (newPyComponent == NULL) return;

    Py_CLEAR(component->pyComponent);
    component->pyComponent = newPyComponent;
    Py_INCREF(component->pyComponent);
    newPyComponent = NULL;
}

PyObject *getPythonComponentAttr(struct PythonComponent *component, const char *attrName) {
    if (component == NULL || component->pyComponent == NULL || attrName == NULL) return NULL;

    PyObject *ret = PyObject_GetAttrString(component->pyComponent, attrName);
    if (ret == NULL) {
        handleException();
    }

    return ret;
}

void setPythonComponentAttr(struct PythonComponent *component, const char *attrName, PyObject *attr) {
    if (component == NULL || component->pyComponent == NULL || attrName == NULL) return;

    if (attr == NULL) {
        attr = Py_None;
    }

    if (PyObject_HasAttrString(component->pyComponent, attrName) == 0) {
        warning_log(
            "[PythonComponent]: Setting attribute \"%s\" that did not exist previously. It may be misspelled.",
            attrName
        );
    }

    if (PyObject_SetAttrString(component->pyComponent, attrName, attr) == -1) {
        handleException();
    }
}