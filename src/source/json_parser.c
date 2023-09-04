#include <string.h>
#include <json.h>

#include "logger.h"
#include "json_parser.h"
#include "python/py3dgameobject.h"
#include "python/py3dresourcemanager.h"
#include "resources/shader.h"
#include "resources/python_script.h"
#include "python/py3dcomponent.h"
#include "python/python_util.h"
#include "math/vector3.h"
#include "math/quaternion.h"

static PyObject *createPyDictFromJsonObject(json_object *json);
static PyObject *createPyListFromJsonArray(json_object *json);

static json_object *fetchProperty(json_object *parent, const char *key, json_type type) {
    if (parent == NULL || key == NULL) return NULL;

    json_object *ret = json_object_object_get(parent, key);
    if (ret == NULL) return NULL;

    if (!json_object_is_type(ret, type)) return NULL;

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

static PyObject *parseVector3(json_object *json) {
    if (json == NULL || !json_object_is_type(json, json_type_object)) return NULL;

    json_object *x = fetchProperty(json, "x", json_type_double);
    if (x == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Vector3 has no x component");
        return NULL;
    }

    json_object *y = fetchProperty(json, "y", json_type_double);
    if (y == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Vector3 has no y component");
        return NULL;
    }

    json_object *z = fetchProperty(json, "z", json_type_double);
    if (z == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Vector3 has no z component");
        return NULL;
    }

    return (PyObject *) Py3dVector3_New(
        (float) json_object_get_double(x),
        (float) json_object_get_double(y),
        (float) json_object_get_double(z)
    );
}

static PyObject *parseQuaternion(json_object *json) {
    if (json == NULL || !json_object_is_type(json, json_type_object)) return NULL;

    json_object *x = fetchProperty(json, "x", json_type_double);
    if (x == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Quaternion has no x component");
        return NULL;
    }

    json_object *y = fetchProperty(json, "y", json_type_double);
    if (y == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Quaternion has no y component");
        return NULL;
    }

    json_object *z = fetchProperty(json, "z", json_type_double);
    if (z == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Quaternion has no z component");
        return NULL;
    }

    json_object *w = fetchProperty(json, "w", json_type_double);
    if (w == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Quaternion has no w component");
        return NULL;
    }

    return (PyObject *) Py3dQuaternion_New(
        (float) json_object_get_double(x),
        (float) json_object_get_double(y),
        (float) json_object_get_double(z),
        (float) json_object_get_double(w)
    );
}

static PyObject *createPyValueFromJsonObject(json_object *json) {
    switch (json_object_get_type(json)) {
        case json_type_int:
            return PyLong_FromLong(json_object_get_int(json));
        case json_type_double:
            return PyFloat_FromDouble(json_object_get_double(json));
        case json_type_string:
            return PyUnicode_FromString(json_object_get_string(json));
        case json_type_boolean:
            return PyBool_FromLong(json_object_get_boolean(json));
        case json_type_null:
            Py_RETURN_NONE;
        case json_type_array:
            return createPyListFromJsonArray(json);
        case json_type_object:
            return createPyDictFromJsonObject(json);
    }
}

static PyObject *createPyDictFromJsonObject(json_object *json) {
    if (json == NULL || json_object_is_type(json, json_type_object) == 0) Py_RETURN_NONE;

    PyObject *ret = PyDict_New();
    if (ret == NULL) return NULL;

    json_object_object_foreach(json, key, val) {
        PyDict_SetItemString(ret, key, createPyValueFromJsonObject(val));
    }

    return ret;
}

static PyObject *createPyListFromJsonArray(json_object *json) {
    if (json == NULL || json_object_is_type(json, json_type_array) == 0) Py_RETURN_NONE;

    // TODO: unsigned to signed cast based on input from a file ... security problem?
    Py_ssize_t array_length = (Py_ssize_t) json_object_array_length(json);

    PyObject *ret = PyList_New(array_length);
    if (ret == NULL) return NULL;

    for(Py_ssize_t i = 0; i < array_length; ++i) {
        json_object *curJson = json_object_array_get_idx(json, i);
        PyList_SetItem(ret, i, createPyValueFromJsonObject(curJson));
    }

    return ret;
}

static bool callPythonComponentParse(struct Py3dComponent *component, PyObject *data, struct Py3dResourceManager *rm) {
    if (
        Py3dComponent_Check((PyObject *) component) != 1 ||
        data == NULL ||
        Py3dResourceManager_Check((PyObject *) rm) != 1
    ) return false;

    PyObject *pyParse = PyObject_GetAttrString((PyObject *) component, "parse");
    if (pyParse == NULL) {
        handleException();

        return false;
    }

    PyObject *parseArgs = Py_BuildValue("(OO)", data, rm);
    PyObject *parseRet = PyObject_Call(pyParse, parseArgs, NULL);
    if (parseRet == NULL) {
        handleException();
        Py_CLEAR(parseArgs);
        Py_CLEAR(pyParse);

        return false;
    }

    Py_CLEAR(parseRet);
    Py_CLEAR(parseArgs);
    Py_CLEAR(pyParse);

    return true;
}

static bool parsePythonComponent(
    struct Py3dComponent *pyComponent,
    json_object *json,
    struct Py3dResourceManager *resourceManager
) {
    if (
        json == NULL ||
        Py3dComponent_Check((PyObject *) pyComponent) != 1 ||
        Py3dResourceManager_Check((PyObject *) resourceManager) != 1
    ) return false;

    PyObject *parsedData = createPyDictFromJsonObject(json);
    if (parsedData == NULL) {
        critical_log("[JsonParser]: Failed to parse attributes for Component");
        handleException();
        return false;
    }

    if (!callPythonComponentParse(pyComponent, parsedData, resourceManager)) {
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
    struct Py3dScene *scene,
    struct Py3dResourceManager *resourceManager
) {
    if (
        json == NULL ||
        rootPtr == NULL ||
        (*rootPtr) != NULL ||
        Py3dResourceManager_Check((PyObject *) resourceManager) != 1
    ) return false;

    json_object *json_name = fetchProperty(json, "name", json_type_string);
    if (json_name == NULL) return false;

    json_object *json_components_array = fetchProperty(json, "components", json_type_array);
    if (json_components_array == NULL) return false;

    json_object *json_children_array = fetchProperty(json, "children", json_type_array);
    if (json_children_array == NULL) return false;

    json_object *json_enabled = fetchProperty(json, "enabled", json_type_boolean);
    json_object *json_visible = fetchProperty(json, "visible", json_type_boolean);

    json_object *json_position = fetchProperty(json, "position", json_type_object);
    json_object *json_orientation = fetchProperty(json, "orientation", json_type_object);
    json_object *json_scale = fetchProperty(json, "scale", json_type_object);

    struct Py3dGameObject *newGO = NULL;
    newGO = (struct Py3dGameObject *) Py3dGameObject_New(scene);
    if (newGO == NULL) return false;

    const char *gameObjectName = json_object_get_string(json_name);
    Py3dGameObject_SetNameCStr(newGO, gameObjectName);
    if (json_enabled != NULL) {
        Py3dGameObject_EnableBool(newGO, json_object_get_boolean(json_enabled));
    }
    if (json_visible != NULL) {
        Py3dGameObject_MakeVisibleBool(newGO, json_object_get_boolean(json_visible));
    }
    if (json_position != NULL) {
        PyObject *newPos = parseVector3(json_position);
        if (newPos == NULL) {
            warning_log("[JsonParser]: Could not parse position of Game Object with name \"%s\"", gameObjectName);
            handleException();
            Py_CLEAR(newPos);
        } else {
            PyObject *ret = PyObject_CallMethod((PyObject *) newGO, "set_position", "(O)", newPos);
            if (ret == NULL) {
                warning_log("[JsonParser]: Could not set position of Game Object with name \"%s\"", gameObjectName);
                handleException();
            }
            Py_CLEAR(ret);
        }
    }
    if (json_orientation != NULL) {
        PyObject *newOrientation = parseQuaternion(json_orientation);
        if (newOrientation == NULL) {
            warning_log("[JsonParser]: Could not parse orientation of Game Object with name \"%s\"", gameObjectName);
            handleException();
            Py_CLEAR(newOrientation);
        } else {
            PyObject *ret = PyObject_CallMethod((PyObject *) newGO, "set_orientation", "(O)", newOrientation);
            if (ret == NULL) {
                warning_log("[JsonParser]: Could not set orientation of Game Object with name \"%s\"", gameObjectName);
                handleException();
            }
            Py_CLEAR(ret);
        }
    }
    if (json_scale != NULL) {
        PyObject *newScale = parseVector3(json_scale);
        if (newScale == NULL) {
            warning_log("[JsonParser]: Could not parse scale of Game Object with name \"%s\"", gameObjectName);
            handleException();
            Py_CLEAR(newScale);
        } else {
            PyObject *ret = PyObject_CallMethod((PyObject *) newGO, "set_scale", "(O)", newScale);
            if (ret == NULL) {
                warning_log("[JsonParser]: Could not set scale of Game Object with name \"%s\"", gameObjectName);
                handleException();
            }
            Py_CLEAR(ret);
        }
    }

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
        struct BaseResource *pyScript = Py3dResourceManager_GetResource(resourceManager,typeName);
        if (!isResourceTypePythonScript(pyScript)) continue;

        createPythonComponent((struct PythonScript *) pyScript, &pyComponent);
        Py3dGameObject_AttachComponentInC(newGO, pyComponent);

        if (!parsePythonComponent(pyComponent, cur_component_json, resourceManager)) {
            error_log("%s", "[JsonParser]: Python component failed to parse.");
            Py3dGameObject_DetachComponentInC(newGO, pyComponent);
        }

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

        parseGameObject(cur_child_json, newGO, rootPtr, scene, resourceManager);
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