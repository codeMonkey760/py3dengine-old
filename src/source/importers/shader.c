#include <json.h>
#include <string.h>

#include "logger.h"
#include "resources/shader.h"
#include "importers/shader.h"

void importShader(struct Shader **shaderPtr, const char *fileName) {
    if (shaderPtr == NULL || (*shaderPtr) != NULL || fileName == NULL) return;

    json_object *shader_desc_root = json_object_from_file(fileName);

    json_object *json_type_name = json_object_object_get(shader_desc_root, "type");
    if (json_type_name == NULL || !json_object_is_type(json_type_name, json_type_string)) {
        error_log("%s", "[SceneImporter]: Shader description must have an attribute of type \"string\" called \"type\"");
        return;
    }
    if (strcmp(json_object_get_string(json_type_name), "shader") != 0) {
        error_log("%s", "[SceneImporter]: Shader description type attribute must be set to \"shader\"");
        return;
    }

    json_object *json_name = json_object_object_get(shader_desc_root, "name");
    if (json_name == NULL || !json_object_is_type(json_name, json_type_string)) {
        error_log("%s", "[SceneImporter]: Shader description must have an attribute of type \"string\" called \"name\"");
        return;
    }

    json_object *vertex_shader_file_name = json_object_object_get(shader_desc_root, "vertex_shader_source_file");
    if (vertex_shader_file_name == NULL || !json_object_is_type(vertex_shader_file_name, json_type_string)) {
        error_log("%s", "[SceneImporter]: Shader description must have an attribute of type \"string\" called \"vertex_shader_source_file\"");
        return;
    }

    json_object *fragment_shader_file_name = json_object_object_get(shader_desc_root, "fragment_shader_source_file");
    if (fragment_shader_file_name == NULL || !json_object_is_type(fragment_shader_file_name, json_type_string)) {
        error_log("%s", "[SceneImporter]: Shader description must have an attribute of type \"string\" called \"fragment_shader_source_file\"");
        return;
    }

    struct Shader *newShader = NULL;
    allocShader(&newShader);
    if (newShader == NULL) return;

    initShaderFromFiles(
            newShader,
            json_object_get_string(vertex_shader_file_name),
            json_object_get_string(fragment_shader_file_name)
    );
    setResourceName((struct BaseResource *) newShader, json_object_get_string(json_name));

    (*shaderPtr) = newShader;
    newShader = NULL;

    json_object_put(shader_desc_root);
    shader_desc_root = NULL;
}
