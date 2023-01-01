#include <stdlib.h>

#include "logger.h"
#include "components/base_component.h"
#include "custom_string.h"

#define COMPONENT_TYPE_INVALID 0

void initializeBaseComponent(struct BaseComponent *component) {
    if (component == NULL) return;

    component->_type = COMPONENT_TYPE_INVALID;
    component->_typeName = NULL;
    component->_name = NULL;
    component->_owner = NULL;
    component->update = NULL;
    component->render = NULL;
    component->resize = NULL;
    component->delete = NULL;
}

void finalizeBaseComponent(struct BaseComponent *component) {
    if (component == NULL) return;

    deleteString(&component->_typeName);
    deleteString(&component->_name);
}

void deleteComponent(struct BaseComponent **componentPtr) {
    if (componentPtr == NULL || (*componentPtr) == NULL) return;

    struct BaseComponent *component = (*componentPtr);
    if (component->delete == NULL) {
        critical_log("%s", "[BaseComponent]: Tried to delete component with no virtual destructor. Memory leak certain.");
        return;
    }

    component->delete(componentPtr);
    component = NULL;
}

struct String *getComponentName(struct BaseComponent *component) {
    if (component == NULL) return NULL;

    return component->_name;
}

void setComponentName(struct BaseComponent *component, const char *newName) {
    if (component == NULL) return;

    if (newName == NULL) {
        deleteString(&component->_name);
        return;
    }

    if (component->_name == NULL) {
        allocString(&component->_name, newName);
    } else {
        setChars(component->_name, newName);
    }
}

struct String *getComponentTypeName(struct BaseComponent *component) {
    if (component == NULL) return NULL;

    return component->_typeName;
}

struct GameObject *getComponentOwner(struct BaseComponent *component) {
    if (component == NULL) return NULL;

    return component->_owner;
}