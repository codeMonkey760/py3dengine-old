#include <stdlib.h>

#include "Components/base_component.h"

void finalizeBaseComponent(struct BaseComponent *component) {
    if (component == NULL) return;

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