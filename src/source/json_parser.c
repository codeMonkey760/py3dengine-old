#include <string.h>
#include <json.h>

#include "logger.h"
#include "util.h"
#include "json_parser.h"
#include "game_object.h"
#include "resource_manager.h"
#include "resources/shader.h"
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
                value = PyBool_FromLong(json_object_get_boolean(val));
                break;
            case json_type_null:
                value = Py_NewRef(Py_None);
                break;
            case json_type_array:
                // TODO: implement this
                value = Py_NewRef(Py_None);
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

bool parseTransformComponent(json_object *json, PyObject *component) {
    if (json == NULL || component == NULL || Py3dTransform_Check(component) != 1) return false;

    struct Py3dTransform *transform = (struct Py3dTransform *) component;

    float dataBuffer[4];
    memset(dataBuffer, 0, sizeof(float) * 4);

    if (parseVec(json, "position", dataBuffer, 3) == false) return false;
    Vec3Copy(transform->position, dataBuffer);

    if (parseVec(json, "orientation", dataBuffer, 4) == false) return false;
    QuaternionCopy(transform->orientation, dataBuffer);

    if (parseVec(json, "scale", dataBuffer, 3) == false) return false;
    Vec3Copy(transform->scale, dataBuffer);

    transform->matrixCacheDirty = true;
    transform->viewMatrixCacheDirty = true;

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

    Py3dComponent_SetNameCStr(pyComponent, json_object_get_string(json_name));

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
    struct Py3dGameObject *parent,
    struct Py3dGameObject **rootPtr,
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

    struct Py3dGameObject *newGO = NULL;
    newGO = (struct Py3dGameObject *) Py3dGameObject_New();
    if (newGO == NULL) return false;

    const char *gameObjectName = json_object_get_string(json_name);
    Py3dGameObject_SetNameCStr(newGO, gameObjectName);
    PyObject *transform = Py3dGameObject_GetTransform(newGO, NULL);
    parseTransformComponent(json_transform, transform);
    Py_CLEAR(transform);

    size_t json_components_array_length = json_object_array_length(json_components_array);
    for (size_t i = 0; i < json_components_array_length; ++i) {
        json_object *cur_component_json = json_object_array_get_idx(json_components_array, i);
        if (cur_component_json == NULL || !json_object_is_type(cur_component_json, json_type_object)) {
            error_log(
                    "[JsonParser]: Could not parse component of Game Object with name \"%s\"",
                    gameObjectName
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

        PyObject *attachComponentArgs = Py_BuildValue("(O)", pyComponent);
        PyObject *attachComponentRet = Py3dGameObject_AttachComponent(newGO, attachComponentArgs, NULL);
        if (attachComponentRet == NULL) {
            error_log("%s", "[JsonParser]: Component failed to attach. Discarding it.");
            handleException();
        }

        Py_CLEAR(attachComponentRet);
        Py_CLEAR(attachComponentArgs);
        Py_CLEAR(pyComponent);
    }

    size_t json_children_array_length = json_object_array_length(json_children_array);
    for (size_t i = 0; i < json_children_array_length; ++i) {
        json_object *cur_child_json = json_object_array_get_idx(json_children_array, i);
        if (cur_child_json == NULL || !json_object_is_type(cur_child_json, json_type_object)) {
            error_log(
                    "[JsonParser]: Could not parse child of Game Object with name \"%s\"",
                    gameObjectName
            );

            continue;
        }

        parseGameObject(cur_child_json, newGO, rootPtr, resourceManager);
    }

    if (parent != NULL) {
        PyObject *attachChildArgs = Py_BuildValue("(O)", newGO);
        PyObject *attachChildRet = Py3dGameObject_AttachChild(parent, attachChildArgs, NULL);
        if (attachChildRet == NULL) {
            error_log("%s", "[JsonParser]: Game Object failed to attach to its parent. Discarding it");
            handleException();
        }
        Py_CLEAR(attachChildRet);
        Py_CLEAR(attachChildArgs);
        Py_CLEAR(newGO);
    } else {
        (*rootPtr) = newGO;
        newGO = NULL;
    }

    return true;
}