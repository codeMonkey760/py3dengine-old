#include <string.h>
#include <json-c/json.h>

#include "logger.h"
#include "util.h"
#include "json_parser.h"
#include "game_object.h"
#include "resource_manager.h"
#include "resources/model.h"
#include "resources/shader.h"
#include "resources/material.h"
#include "resources/python_script.h"
#include "components/component_factory.h"
#include "components/base_component.h"
#include "components/model_renderer_component.h"
#include "python/py3dcomponent.h"
#include "python/py3dtransform.h"
#include "python/python_util.h"

#define TYPE_NAME_MAX_SIZE 64

static json_object *fetchProperty(json_object *parent, const char *key, json_type type) {
    if (parent == NULL || key == NULL) return NULL;

    json_object *ret = json_object_object_get(parent, key);
    if (ret == NULL) {
        error_log("[JsonParser]: Could not fetch field named \"%s\"", key);

        return NULL;
    }
    if (!json_object_is_type(ret, type)) {
        error_log(
            "[JsonParser]: Type check failure. Expected \"%s\" to be \"%s\", but got \"%s\" instead",
            key,
            json_type_to_name(type),
            json_type_to_name(json_object_get_type(ret))
        );

        return NULL;
    }

    return ret;
}

static bool parseVec(json_object *json, const char *name, float dst[4], size_t vecSize) {
    json_object *vec = fetchProperty(json, name, json_type_object);
    if (vec == NULL) return false;

    char elements[8] = {'x', 0, 'y', 0, 'z', 0, 'w', 0};
    for (int i = 0; i < vecSize; ++i) {
        char *element_name = elements+(i*2);
        json_object *element = fetchProperty(vec, element_name, json_type_double);
        if (element == NULL) return false;

        dst[i] = (float) json_object_get_double(element);
    }

    return true;
}

static PyObject *createPyDictFromJsonObject(json_object *root) {
    if (root == NULL || json_object_is_type(root, json_type_object) == 0) Py_RETURN_NONE;

    PyObject *ret = PyDict_New();
    if (ret == NULL) Py_RETURN_NONE;

    json_object_object_foreach(root, key, val) {
        PyObject *value = NULL;
        switch (json_object_get_type(val)) {
            case json_type_int:
                value = PyLong_FromLong(json_object_get_int(val));
                break;
            case json_type_double:
                value = PyFloat_FromDouble(json_object_get_double(val));
                break;
            case json_type_string:
                value = PyUnicode_FromString(json_object_get_string(val));
                break;
            case json_type_boolean:
                // TODO: double check this
                value = PyBool_FromLong(json_object_get_boolean(val));
                break;
            case json_type_null:
                Py_INCREF(Py_None);
                value = Py_None;
                break;
            case json_type_array:
                // TODO: implement this
                Py_INCREF(Py_None);
                value = Py_None;
                break;
            case json_type_object:
                value = createPyDictFromJsonObject(val);
                break;
        }

        PyDict_SetItemString(ret, key, value);
        Py_CLEAR(value);
    }

    return ret;
}

bool parseModelRendererComponent(struct ModelRendererComponent *component, json_object *json, struct ResourceManager *manager) {
    if (component == NULL || json == NULL || manager == NULL) return false;

    json_object *json_name = fetchProperty(json, "name", json_type_string);
    if (json_name == NULL) return false;

    json_object *json_model_name = fetchProperty(json, "model", json_type_string);
    if (json_model_name == NULL) return false;

    json_object *json_shader_name = fetchProperty(json, "shader", json_type_string);
    if (json_shader_name == NULL) return false;

    json_object *json_material_name = fetchProperty(json, "material", json_type_string);
    if (json_material_name == NULL) return false;

    setComponentName((struct BaseComponent *) component, json_object_get_string(json_name));

    const char *newModelName = json_object_get_string(json_model_name);
    struct BaseResource *newModel = getResource(manager, newModelName);
    if (newModel != NULL || !isResourceTypeModel(newModel)) {
        setModelRendererComponentModel(component, (struct Model *) newModel);
    } else {
        warning_log("[JsonParser]: Model resource look up failed for \"%s\" while parsing component", newModelName);
    }

    const char *newShaderName = json_object_get_string(json_shader_name);
    struct BaseResource *newShader = getResource(manager, newShaderName);
    if (newShader != NULL || !isResourceTypeShader(newShader)) {
        setModelRendererComponentShader(component, (struct Shader *) newShader);
    } else {
        warning_log("[JsonParser]: Shader resource look up failed for \"%s\" while parsing component", newShaderName);
    }

    const char *newMaterialName = json_object_get_string(json_material_name);
    struct BaseResource *newMaterial = getResource(manager, newMaterialName);
    if (newMaterial != NULL || !isResourceTypeMaterial(newMaterial)) {
        setModelRendererComponentMaterial(component, (struct Material *) newMaterial);
    } else {
        warning_log("[JsonParser]: Material resource look up failed for \"%s\" while parsing component", newMaterialName);
    }

    return true;
}

bool parseTransformComponent(json_object *json, struct Py3dTransform *component) {
    if (json == NULL || component == NULL) return false;

    float dataBuffer[4];
    memset(dataBuffer, 0, sizeof(float) * 4);

    if (parseVec(json, "position", dataBuffer, 3) == false) return false;
    Vec3Copy(component->position, dataBuffer);

    if (parseVec(json, "orientation", dataBuffer, 4) == false) return false;
    QuaternionCopy(component->orientation, dataBuffer);

    if (parseVec(json, "scale", dataBuffer, 3) == false) return false;
    Vec3Copy(component->scale, dataBuffer);

    component->matrixCacheDirty = true;
    component->viewMatrixCacheDirty = true;

    return true;
}

static bool parsePythonComponent(
    struct Py3dComponent *pyComponent,
    json_object *json,
    struct ResourceManager *resourceManager
) {
    if (json == NULL || pyComponent == NULL || resourceManager == NULL) return false;

    json_object *json_name = fetchProperty(json, "name", json_type_string);
    if (json_name == NULL) return false;

    PyObject *pyName = PyUnicode_FromString(json_object_get_string(json_name));
    if (pyName == NULL) return false;

    // TODO: the previous value of pyComponent->name is probably leaked here
    // AVOID direct parameter assignments like this!!!!
    ((struct Py3dComponent *) pyComponent)->name = pyName;

    if (PyObject_HasAttrString((PyObject *) pyComponent, "parse") != 1) {
        error_log(
            "[JsonParser]: Component named \"%s\" did not have a parse attribute",
            json_object_get_string(json_name)
        );
        return false;
    }
    PyObject *pyParse = PyObject_GetAttrString((PyObject *) pyComponent, "parse");
    if (pyParse == NULL) {
        critical_log(
            "[JsonParser]: Failed to retrieve \"parse\" attribute from \"%s\"",
            json_object_get_string(json_name)
        );
        handleException();
        return false;
    }
    if (PyCallable_Check(pyParse) != 1) {
        error_log(
            "[JsonParser]: \"parse\" attribute for component \"%s\" must be callable",
            json_object_get_string(json_name)
        );
        Py_CLEAR(pyParse);
        return false;
    }

    PyObject *parsedData = createPyDictFromJsonObject(json);
    if (parsedData == NULL) {
        critical_log(
            "[JsonParser]: Failed to parse attributes for Component named \"%s\"",
            json_object_get_string(json_name)
        );
        handleException();
        Py_CLEAR(pyParse);
        return false;
    }

    PyObject *parseArgs = Py_BuildValue("(NN)", parsedData, resourceManager->py3dResourceManager);
    PyObject *parseRet = PyObject_Call(pyParse, parseArgs, NULL);
    if (parseRet == NULL) {
        error_log(
            "[JsonParser]: \"parse\" raised exception for Component named \"%s\"",
            json_object_get_string(json_name)
        );
        handleException();
        Py_CLEAR(parsedData);
        Py_CLEAR(parseArgs);
        Py_CLEAR(pyParse);
        return false;
    }

    if (!Py_IsNone(parseRet)) {
        warning_log(
            "[JsonParser]: \"parse\" returned a value for Component named \"%s\", which is weird",
            json_object_get_string(json_name)
        );
    }

    Py_CLEAR(parseRet);
    Py_CLEAR(parseArgs);
    Py_CLEAR(parsedData);
    Py_CLEAR(pyParse);

    return true;
}

bool parseComponentByType(
    struct BaseComponent *component,
    const char *typeName,
    json_object *json,
    struct ResourceManager *resourceManager
) {
    if (component == NULL || typeName == NULL || json == NULL || resourceManager == NULL) return false;

    if (strncmp(COMPONENT_TYPE_NAME_MODEL_RENDERER, typeName, TYPE_NAME_MAX_SIZE) == 0) {
        return parseModelRendererComponent((struct ModelRendererComponent *) component, json, resourceManager);
    }

    return false;
}

bool parseGameObject(
        json_object *json,
        struct GameObject *parent,
        struct GameObject **rootPtr,
        struct ResourceManager *resourceManager
) {
    if (json == NULL || rootPtr == NULL || (*rootPtr) != NULL || resourceManager == NULL) return false;

    json_object *json_name = fetchProperty(json, "name", json_type_string);
    if (json_name == NULL) return false;

    json_object *json_transform = fetchProperty(json, "transform", json_type_object);
    if (json_transform == NULL) return false;

    json_object *json_components_array = fetchProperty(json, "components", json_type_array);
    if (json_components_array == NULL) return false;

    json_object *json_children_array = fetchProperty(json, "children", json_type_array);
    if (json_children_array == NULL) return false;

    struct GameObject *newGO = NULL;
    allocGameObject(&newGO);
    if (newGO == NULL) return false;

    setGameObjectName(newGO, json_object_get_string(json_name));
    parseTransformComponent(json_transform, getGameObjectTransform(newGO));

    size_t json_components_array_length = json_object_array_length(json_components_array);
    for (size_t i = 0; i < json_components_array_length; ++i) {
        json_object *cur_component_json = json_object_array_get_idx(json_components_array, i);
        if (cur_component_json == NULL || !json_object_is_type(cur_component_json, json_type_object)) {
            error_log(
                    "[JsonParser]: Could not parse component of Game Object with name \"%s\"",
                    getGameObjectName(newGO)
            );

            continue;
        }

        json_object *type_name_json = fetchProperty(cur_component_json, "type", json_type_string);
        if (type_name_json == NULL) continue;
        const char *typeName = json_object_get_string(type_name_json);

        struct BaseComponent *newComponent = NULL;
        struct Py3dComponent *pyComponent = NULL;
        componentFactoryCreateComponentFromTypeName(typeName, &newComponent);
        if (newComponent == NULL) {
            struct BaseResource *pyScript = getResource(resourceManager,typeName);
            if (!isResourceTypePythonScript(pyScript)) continue;

            createPythonComponent((struct PythonScript *) pyScript, &pyComponent);
            if (!parsePythonComponent(pyComponent, cur_component_json, resourceManager)) {
                error_log("%s", "[JsonParser]: Python component failed to parse. Discarding it.");
                Py_CLEAR(pyComponent);

                continue;
            }

            attachPyComponent(newGO, pyComponent);
            pyComponent = NULL;
        } else {
            if (!parseComponentByType(newComponent, typeName, cur_component_json, resourceManager)) {
                error_log("%s", "[JsonParser]: Component failed to parse. Discarding it.");
                deleteComponent(&newComponent);

                continue;
            }

            attachComponent(newGO, newComponent);
            newComponent = NULL;
        }
    }

    size_t json_children_array_length = json_object_array_length(json_children_array);
    for (size_t i = 0; i < json_children_array_length; ++i) {
        json_object *cur_child_json = json_object_array_get_idx(json_children_array, i);
        if (cur_child_json == NULL || !json_object_is_type(cur_child_json, json_type_object)) {
            error_log(
                    "[JsonParser]: Could not parse child of Game Object with name \"%s\"",
                    getGameObjectName(newGO)
            );

            continue;
        }

        parseGameObject(cur_child_json, newGO, rootPtr, resourceManager);
    }

    if (parent != NULL) {
        attachChild(parent, newGO);
    } else {
        (*rootPtr) = newGO;
    }
    newGO = NULL;
}