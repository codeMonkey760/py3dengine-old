#include <SOIL/SOIL.h>
#include <stdlib.h>

#include "custom_string.h"
#include "resources/texture.h"

#define RESOURCE_TYPE_TEXTURE 4

bool isResourceTypeTexture(struct BaseResource *resource) {
    if (resource == NULL) return false;

    return resource->_type == RESOURCE_TYPE_TEXTURE && stringEqualsCStr(resource->_typeName, RESOURCE_TYPE_NAME_TEXTURE);
}

