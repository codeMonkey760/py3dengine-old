#include <stdlib.h>
#include <stdbool.h>

#include "components/python_component.h"
#include "rendering_context.h"
#include "custom_string.h"
#include "logger.h"

#define COMPONENT_TYPE_PYTHON 5

static bool isComponentValid(struct BaseComponent *component) {
    if (component == NULL) return false;

    return component->_type == COMPONENT_TYPE_PYTHON;
}

static void update(struct BaseComponent *component, float dt) {
    if (!isComponentValid(component)) return;
}

static void render(struct BaseComponent *component, struct RenderingContext *renderingContext) {
    if  (!isComponentValid(component)) return;
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
    component = NULL;

    finalizeBaseComponent((struct BaseComponent *) (*componentPtr));

    free( (*componentPtr) );
    (*componentPtr) = NULL;
}

void initPythonComponent(struct PythonComponent *component, PyObject *componentType) {
    if (component == NULL || componentType == NULL) return;

    if (PyCallable_Check(componentType) < 0) {
        error_log("%s", "[PythonComponent]: Component type must be callable");

        return;
    }

    PyObject *newPyComponent = PyObject_Call(componentType, PyTuple_New(0), NULL);
    if (newPyComponent == NULL) return;

    // TODO: finish this
}