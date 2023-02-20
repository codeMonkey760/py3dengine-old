#ifndef PY3DENGINE_SPRITE_H
#define PY3DENGINE_SPRITE_H

#include "resources/base_resource.h"

#define RESOURCE_TYPE_NAME_SPRITE "Sprite"

struct Sprite {
    struct BaseResource _base;

    struct Texture *_spriteSheet;
    int _bounds[4]; //left, top, width, height in pixels
};

extern bool isResourceTypeSprite(struct BaseResource *resource);
extern void allocSprite(struct Sprite **spritePtr);
extern void deleteSprite(struct Sprite **spritePtr);

extern bool initSprite(struct Sprite *sprite, struct Texture *newSpriteSheet, int newBounds[4]);

extern struct Texture *getSpriteSheet(struct Sprite *sprite);
extern const int *getSpriteBounds(struct Sprite *sprite);

#endif
