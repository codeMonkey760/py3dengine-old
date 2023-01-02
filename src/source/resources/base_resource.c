#include <stdlib.h>

#include "logger.h"
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

void deleteResource(struct BaseResource **resourcePtr) {
    if (resourcePtr == NULL) return;

    struct BaseResource *resource = (*resourcePtr);
    if (resource->delete == NULL) {
        critical_log("%s", "[BaseResource]: Unable to delete resource. Delete virtual function was NULL. Memory leak certain.");
        return;
    }

    resource->delete(resourcePtr);
    resource = NULL;
}

bool resourceTypesEqual(struct BaseResource *r1, struct BaseResource *r2) {
    if (r1 == NULL || r2 == NULL) return false;

    if (r1->_type != r2->_type) return false;

    return stringEquals(r1->_typeName, r2->_typeName);
}

bool resourceNamesEqual(struct BaseResource *r1, struct BaseResource *r2) {
    if (r1 == NULL || r2 == NULL) return false;

    return stringEquals(r1->_name, r2->_name);
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