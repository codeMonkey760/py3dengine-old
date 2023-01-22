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

    PyObject *parsedData = createPyDictFromJsonObject(json);
    if (parsedData == NULL) {
        critical_log(
            "[JsonParser]: Failed to parse attributes for Component named \"%s\"",
            json_object_get_string(json_name)
        );
        handleException();
        return false;
    }

    if (!Py3dComponent_CallParse(pyComponent, parsedData, resourceManager)) {
        Py_CLEAR(parsedData);
        return false;
    }

    Py_CLEAR(parsedData);
    return true;
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

        struct Py3dComponent *pyComponent = NULL;
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