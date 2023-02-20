#include <stdlib.h>
#include <string.h>

#include "custom_string.h"
#include "resources/sprite.h"

#define RESOURCE_TYPE_SPRITE 6

static void delete(struct BaseResource **resourcePtr) {
    if (resourcePtr == NULL) return;

    if (!isResourceTypeSprite((*resourcePtr))) return;

    deleteSprite((struct Sprite **) resourcePtr);
}

bool isResourceTypeSprite(struct BaseResource *resource) {
    if (resource == NULL) return false;

    return resource->_type == RESOURCE_TYPE_SPRITE && stringEqualsCStr(resource->_typeName, RESOURCE_TYPE_NAME_SPRITE);
}

void allocSprite(struct Sprite **spritePtr) {
    if (spritePtr == NULL || (*spritePtr) == NULL) return;

    struct Sprite *newSprite = calloc(1, sizeof(struct Sprite));
    if (newSprite == NULL) return;

    struct BaseResource *base = (struct BaseResource *) newSprite;
    initializeBaseResource(base);
    base->_type = RESOURCE_TYPE_SPRITE;
    allocString(&base->_typeName, RESOURCE_TYPE_NAME_SPRITE);
    base->delete = delete;
    base = NULL;

    newSprite->_spriteSheet = NULL;
    memset(newSprite->_bounds, 0, 4 * sizeof(int));

    (*spritePtr) = newSprite;
    newSprite = NULL;
}

void deleteSprite(struct Sprite **spritePtr) {
    if (spritePtr == NULL || (*spritePtr) == NULL) return;

    struct Sprite *sprite = (*spritePtr);
    sprite->_spriteSheet = NULL;

    finalizeBaseResource((struct BaseResource *) sprite);

    free(sprite);
    sprite = NULL;
    (*spritePtr) = NULL;
}

bool initSprite(struct Sprite *sprite, struct Texture *newSpriteSheet, int newBounds[4]) {
    if (sprite == NULL || newSpriteSheet == NULL || newBounds == NULL) return false;

    sprite->_spriteSheet = newSpriteSheet;
    for (int i = 0; i < 4; ++i) {
        sprite->_bounds[i] = newBounds[i];
    }

    return true;
}

extern struct Texture *getSpriteSheet(struct Sprite *sprite) {
    if (sprite == NULL) return NULL;

    return sprite->_spriteSheet;
}

extern const int *getSpriteBounds(struct Sprite *sprite) {
    if (sprite == NULL) return NULL;

    return sprite->_bounds;
}
