#include <stdlib.h>

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
    component->parse = NULL;
    component->delete = NULL;
}

void finalizeBaseComponent(struct BaseComponent *component) {
    if (component == NULL) return;

    deleteString(&component->_typeName);
    deleteString(&component->_name);
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