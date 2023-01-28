#include <SOIL/SOIL.h>
#include <glad/gl.h>
#include <stdlib.h>

#include "custom_string.h"
#include "resources/base_resource.h"
#include "resources/texture.h"
#include "logger.h"

#define RESOURCE_TYPE_TEXTURE 4

static void deleteTextureId(struct Texture *texture) {
    if (texture->_id == 0) return;

    glDeleteTextures(1, &texture->_id);
    texture->_id = 0;
}

static void delete(struct BaseResource **resourcePtr) {
    if (resourcePtr == NULL) return;

    if (!isResourceTypeTexture((*resourcePtr))) return;

    deleteTexture((struct Texture **) resourcePtr);
}

bool isResourceTypeTexture(struct BaseResource *resource) {
    if (resource == NULL) return false;

    return resource->_type == RESOURCE_TYPE_TEXTURE && stringEqualsCStr(resource->_typeName, RESOURCE_TYPE_NAME_TEXTURE);
}

void allocTexture(struct Texture **texturePtr) {
    if (texturePtr == NULL || (*texturePtr) != NULL) return;

    struct Texture *newTexture = calloc(1, sizeof(struct Texture));

    struct BaseResource *base = (struct BaseResource *) newTexture;
    initializeBaseResource(base);
    base->_type = RESOURCE_TYPE_TEXTURE;
    allocString(&base->_typeName, RESOURCE_TYPE_NAME_TEXTURE);
    base->delete = delete;
    base = NULL;

    newTexture->_id = 0;
    (*texturePtr) = newTexture;
    newTexture = NULL;
}

void deleteTexture(struct Texture **texturePtr) {
    if (texturePtr == NULL || (*texturePtr) == NULL) return;

    struct Texture *texture = (*texturePtr);
    deleteTextureId(texture);

    finalizeBaseResource((struct BaseResource *) texture);

    free(texture);
    texture = NULL;
    (*texturePtr) = NULL;
}

void initTexture(struct Texture *texture, const char *fileName) {
    if (texture == NULL || fileName == NULL) return;

    unsigned int newId = SOIL_load_OGL_texture(fileName, 4, 0, SOIL_FLAG_MIPMAPS);
    if (newId == 0) {
        error_log("[Texture]: SOIL returned error code when trying to load \"%s\"", fileName);
        return;
    }

    deleteTextureId(texture);
    texture->_id = newId;
}
