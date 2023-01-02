#ifndef PY3DENGINE_TEXTURE_H
#define PY3DENGINE_TEXTURE_H

#define RESOURCE_TYPE_NAME_TEXTURE "Texture"

#include <stdbool.h>

#include "resources/base_resource.h"

struct Texture {
    struct BaseResource base;

    unsigned int _texture;
    int _width;
    int _height;
    int _channelCount;
    int _format;
};

extern bool isResourceTypeTexture(struct BaseResource *);
extern void allocTexture(struct Texture **texturePtr);
extern void deleteTexture(struct Texture **texturePtr);
extern void initTexture(struct Texture *texture, const char *fileName);

#endif
