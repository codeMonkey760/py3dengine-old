#include "importers/sprite_sheet.h"
#include "resources/texture.h"
#include "resources/sprite.h"
#include "importers/texture.h"
#include "resource_manager.h"
#include "logger.h"

static int getIntegerFromJsonArray(json_object *array, size_t index, int *dst) {
    json_object *value = json_object_array_get_idx(array, index);
    if (value == NULL || !json_object_is_type(value, json_type_int)) {
        return 0;
    }

    (*dst) = json_object_get_int(value);
    return 1;
}

void importSprites(struct ResourceManager *manager, json_object *resourceDescriptor) {
    if (manager == NULL || resourceDescriptor == NULL) return;

    json_object *texture_name_json = json_object_object_get(resourceDescriptor, "filename");
    if (texture_name_json == NULL || !json_object_is_type(texture_name_json, json_type_string)) {
        error_log("%s", "[SpriteSheetImporter]: Resource descriptor must have a field named \"filename\" of type string");
        return;
    }
    const char *texture_name = json_object_get_string(texture_name_json);

    json_object *sprites_map = json_object_object_get(resourceDescriptor, "sprites");
    if (sprites_map == NULL || !json_object_is_type(sprites_map, json_type_object)) {
        error_log("%s", "[SpriteSheetImporter]: Resource descriptor must have a field named \"sprites\" of type object");
        return;
    }

    if (json_object_object_length(sprites_map) < 0) {
        error_log("%s", "[SpriteSheetImporter]: \"sprites\" object must have sprites as fields");
        return;
    }

    struct BaseResource *spriteSheetResource = getResource(manager, texture_name);
    struct Texture *spriteSheetTexture = NULL;
    if (!isResourceTypeTexture(spriteSheetResource)) {
        // TODO: what happens if we have a resource with the same name that's not a texture? Nuthin good
        importTexture(&spriteSheetTexture, resourceDescriptor);
        storeResource(manager, (struct BaseResource *) spriteSheetTexture);
    } else {
        spriteSheetTexture = (struct Texture *) spriteSheetResource;
    }
    spriteSheetResource = NULL;

    json_object_object_foreach(sprites_map, sprite_name, dimensions) {
        if (!json_object_is_type(dimensions, json_type_array) || json_object_array_length(dimensions) != 4) {
            error_log("[SpriteSheetImporter]: Sprite with name \"%s\" must be an array with four numbers", sprite_name);
            continue;
        }

        int bounds[4] = {0};
        int importedBounds = 1;
        for (size_t i = 0; i < 4; ++i) {
            if (getIntegerFromJsonArray(dimensions, i, &(bounds[i])) == 0) {
                error_log("[SpriteSheetImporter]: Sprite with name \"%s\" has no int index at \"%d\"", sprite_name, i);
                importedBounds = 0;
                break;
            }
        }

        if (importedBounds == 0) continue;

        struct Sprite *newSprite = NULL;
        allocSprite(&newSprite);
        if (newSprite == NULL) continue;
        setResourceName((struct BaseResource *) newSprite, sprite_name);
        if (!initSprite(newSprite, spriteSheetTexture, bounds)) {
            deleteSprite(&newSprite);
        }
        storeResource(manager, (struct BaseResource *) newSprite);
        newSprite = NULL;
    }
}
