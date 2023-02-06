#include <json.h>
#include <string.h>

#include "logger.h"
#include "resources/texture.h"
#include "importers/texture.h"

void importTexture(struct Texture **texturePtr, json_object *textureDesc) {
    if (texturePtr == NULL || (*texturePtr) != NULL || textureDesc == NULL) return;

    json_object *json_name = json_object_object_get(textureDesc, "name");
    if (json_name == NULL || !json_object_is_type(json_name, json_type_string)) {
        error_log("%s", "[TextureImporter]: Texture descriptor must contain a \"name\" field of type string");
        return;
    }

    json_object *json_texture_path = json_object_object_get(textureDesc, "filename");
    if (json_texture_path == NULL || !json_object_is_type(json_texture_path, json_type_string)) {
        error_log("%s", "[TextureImporter]: Texture descriptor must contain a \"filename\" field of type string");
        return;
    }

    struct Texture *newTexture = NULL;
    allocTexture(&newTexture);
    if (newTexture == NULL) return;

    setResourceName((struct BaseResource *) newTexture, json_object_get_string(json_name));
    initTexture(newTexture, json_object_get_string(json_texture_path));

    json_object *json_tex_param_map = json_object_object_get(textureDesc, "texture_parameters");
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

    (*texturePtr) = newTexture;
    newTexture = NULL;
}
