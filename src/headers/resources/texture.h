#ifndef PY3DENGINE_RESOURCES_TEXTURE_H
#define PY3DENGINE_RESOURCES_TEXTURE_H

#define RESOURCE_TYPE_NAME_TEXTURE "Texture"

#include <stdbool.h>

#include "resources/base_resource.h"

struct Texture {
    struct BaseResource base;

    unsigned int _id;
    int _width;
    int _height;
};

extern bool isResourceTypeTexture(struct BaseResource *resource);
extern void allocTexture(struct Texture **texturePtr);
extern void deleteTexture(struct Texture **texturePtr);

extern void initTexture(struct Texture *texture, const char *fileName);
extern void setTextureParam(struct Texture *texture, const char *paramName, const char *paramValue);

extern unsigned int getTextureId(struct Texture *texture);
extern int getTextureWidth(struct Texture *texture);
extern int getTextureHeight(struct Texture *texture);

#endif
