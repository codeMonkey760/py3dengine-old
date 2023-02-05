#include <json.h>

#include "logger.h"
#include "resources/texture.h"
#include "importers/texture.h"

void importTexture(struct Texture **texturePtr, const char *descriptorPath) {
    if (texturePtr == NULL || (*texturePtr) != NULL || descriptorPath == NULL) return;

    json_object *texture_desc = json_object_from_file(descriptorPath);

    json_object *json_type = json_object_object_get(texture_desc, "type");
    if (json_type == NULL || !json_object_is_type(json_type, json_type_string)) {
        error_log("%s", "[TextureImporter]: Texture descriptor must contain a \"type\" field of type string");
        json_object_put(texture_desc);
        texture_desc = NULL;
        return;
    }

    json_object *json_texture_path = json_object_object_get(texture_desc, "filename");
    if (json_texture_path == NULL || !json_object_is_type(json_texture_path, json_type_string)) {
        error_log("%s", "[TextureImporter]: Texture descriptor must contain a \"filename\" field of type string");
        json_object_put(texture_desc);
        texture_desc = NULL;
        return;
    }

    struct Texture *newTexture = NULL;
    allocTexture(&newTexture);
    if (newTexture == NULL) {
        json_object_put(texture_desc);
        texture_desc = NULL;
        return;
    }

    initTexture(newTexture, json_object_get_string(json_texture_path));

    json_object *json_tex_param_map = json_object_object_get(texture_desc, "texture_parameters");
    if (json_tex_param_map != NULL && json_object_is_type(json_tex_param_map, json_type_object)) {
        json_object_object_foreach(json_tex_param_map, key, val) {
            if (!json_object_is_type(val, json_type_string)) {
                error_log(
                    "[TextureImporter]: Texture parameter \"%s\", should be a string representing a valid GLenum value",
                    key
                );
                continue;
            }

            setTextureParam(newTexture, key, json_object_get_string(val));
        }
    }

    json_object_put(texture_desc);
    texture_desc = NULL;
    (*texturePtr) = newTexture;
    newTexture = NULL;
}
