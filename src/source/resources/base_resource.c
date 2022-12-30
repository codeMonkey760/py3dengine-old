#include <stdlib.h>

#include "resources/base_resource.h"
#include "custom_string.h"

#define RESOURCE_TYPE_INVALID 0

void initializeBaseResource(struct BaseResource *resource) {
    if (resource == NULL) return;

    resource->_type = RESOURCE_TYPE_INVALID;
    resource->_typeName = NULL;
    resource->_name = NULL;
    resource->delete = NULL;
}

void finalizeBaseResource(struct BaseResource *resource) {
    if (resource == NULL) return;

    deleteString(&resource->_typeName);
    deleteString(&resource->_name);
}

struct String *getResourceName(struct BaseResource *resource) {
    if (resource == NULL) return NULL;

    return resource->_name;
}

void setResourceName(struct BaseResource *resource, const char *newName) {
    if (resource == NULL) return;

    if (newName == NULL) {
        deleteString(&resource->_name);
        return;
    }

    if (resource->_name == NULL) {
        allocString(&resource->_name, newName);
    } else {
        setChars(resource->_name, newName);
    }
}

struct String *getResourceTypeName(struct BaseResource *resource) {
    if (resource == NULL) return NULL;

    return resource->_typeName;
}