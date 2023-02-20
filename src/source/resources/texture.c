#include <SOIL/SOIL.h>
#include <glad/gl.h>
#include <stdlib.h>
#include <string.h>

#include "custom_string.h"
#include "resources/base_resource.h"
#include "resources/texture.h"
#include "logger.h"

#define RESOURCE_TYPE_TEXTURE 4

static void deleteTextureId(struct Texture *texture) {
    if (texture->_id == 0) return;

    glDeleteTextures(1, &texture->_id);
    texture->_id = 0;
    texture->_width = 0;
    texture->_height = 0;
}

static void delete(struct BaseResource **resourcePtr) {
    if (resourcePtr == NULL) return;

    if (!isResourceTypeTexture((*resourcePtr))) return;

    deleteTexture((struct Texture **) resourcePtr);
}

static GLenum convertParamName(const char *pName) {
    if (strcmp(pName, "GL_TEXTURE_MIN_FILTER") == 0) return GL_TEXTURE_MIN_FILTER;
    if (strcmp(pName, "GL_TEXTURE_MAG_FILTER") == 0) return GL_TEXTURE_MAG_FILTER;
    if (strcmp(pName, "GL_TEXTURE_WRAP_T") == 0) return GL_TEXTURE_WRAP_T;
    if (strcmp(pName, "GL_TEXTURE_WRAP_S") == 0) return GL_TEXTURE_WRAP_S;

    return GL_INVALID_ENUM;
}

static GLint convertParamValue(const char *pVal) {
    if (strcmp(pVal, "GL_NEAREST") == 0) return GL_NEAREST;
    if (strcmp(pVal, "GL_LINEAR") == 0) return GL_LINEAR;
    if (strcmp(pVal, "GL_REPEAT") == 0) return GL_REPEAT;
    if (strcmp(pVal, "GL_MIRRORED_REPEAT") == 0) return GL_MIRRORED_REPEAT;

    return GL_INVALID_VALUE;
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
    newTexture->_width = 0;
    newTexture->_height = 0;
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

    int newWidth = 0, newHeight = 0;
    unsigned char *imageData = SOIL_load_image(fileName, &newWidth, &newHeight, NULL, SOIL_LOAD_RGBA);
    if (imageData == NULL) {
        error_log("[Texture]: SOIL failed to load image data from \"%s\"", fileName);
        return;
    }

    GLuint newId = 0;
    glGenTextures(1, &newId);
    if (newId == 0) {
        error_log("[Texture]: OpenGL could not allocate texture object", fileName);
        free(imageData);
        imageData = NULL;
        return;
    }

    glBindTexture(GL_TEXTURE_2D, newId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    free(imageData);
    imageData = NULL;
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        error_log("[Texture]: OpenGL generated an error with code \"%d\" while trying to load \"%s\"", error, fileName);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &newId);
        return;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    deleteTextureId(texture);
    texture->_id = newId;
    texture->_width = newWidth;
    texture->_height = newHeight;
}

void setTextureParam(struct Texture *texture, const char *paramName, const char *paramValue) {
    if (texture == NULL || texture->_id == 0 || paramName == NULL || paramValue == NULL) return;

    GLenum pName = convertParamName(paramName);
    if (pName == GL_INVALID_ENUM) {
        error_log("[Texture]: Cannot set texture parameter \"%s\". Unsupported parameter", paramName);
        return;
    }

    GLint pVal = convertParamValue(paramValue);
    if (pVal == GL_INVALID_VALUE) {
        error_log("[Texture]: Cannot set value \"%s\". Unsupported value", paramValue);
        return;
    }

    glTextureParameteri(texture->_id, pName, pVal);
}

unsigned int getTextureId(struct Texture *texture) {
    if (texture == NULL) return 0;

    return texture->_id;
}

int getTextureWidth(struct Texture *texture) {
    if (texture == NULL) return 0;

    return texture->_width;
}

int getTextureHeight(struct Texture *texture) {
    if (texture == NULL) return 0;

    return texture->_height;
}
