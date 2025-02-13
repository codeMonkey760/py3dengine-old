#include "logger.h"
#include "python/python_util.h"
#include "python/py3dgameobject.h"
#include "python/py3dcomponent.h"
#include "python/py3dcollisionevent.h"
#include "python/py3dcontactpoint.h"
#include "python/py3drenderingcontext.h"
#include "python/py3dscene.h"
#include "math/vector3.h"
#include "math/quaternion.h"

static PyObject *convertToPyString(PyObject *obj) {
    PyObject *ret = PyObject_Str(obj);
    Py_XDECREF(obj);

    return ret;
}

void handleException() {
    if (PyErr_Occurred() == NULL) return;

    PyObject *type = NULL, *value = NULL, *traceback = NULL;
    PyErr_Fetch(&type, &value, &traceback);

    type = convertToPyString(type);
    value = convertToPyString(value);

    error_log(
            "[Python]: Operation yielded an error:\n"
            "Type: %s\n"
            "Value: %s",
            (type != NULL) ? PyUnicode_AsUTF8(type) : "NULL",
            (value != NULL) ? PyUnicode_AsUTF8(PyObject_Str(value)): "NULL"
    );

    Py_XDECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);
}

static PyObject *interesting_types[] = {
    (PyObject *) &Py3dGameObject_Type,
    (PyObject *) &Py3dComponent_Type,
    (PyObject *) &Py3dCollisionEvent_Type,
    (PyObject *) &Py3dContactPoint_Type,
    (PyObject *) &Py3dRenderingContext_Type,
    (PyObject *) &Py3dVector3_Type,
    (PyObject *) &Py3dQuaternion_Type,
    (PyObject *) &Py3dScene_Type,
};

static bool isObjectInteresting(PyObject *obj) {
    int num_types = sizeof(interesting_types) / sizeof(PyObject *);

    for (int i = 0; i < num_types; ++i) {
        if (PyObject_IsInstance(obj, (PyObject *) interesting_types[i])) {
            return true;
        }
    }

    return false;
}

static PyObject *callGCMethod(const char *callableName, PyObject *args) {
    PyObject *gcModule = PyImport_ImportModule("gc");
    if (gcModule == NULL) {
        critical_log("[Python]: Failed to import module \"gc\"");
        handleException();
        return NULL;
    }

    PyObject *callable = PyObject_GetAttrString(gcModule, callableName);
    if (callable == NULL) {
        critical_log("[Python]: Module \"gc\" has no attribute \"%s\"", callableName);
        handleException();
        Py_CLEAR(gcModule);
        return NULL;
    } else if (!PyCallable_Check(callable)) {
        critical_log("[Python]: \"gc.%s\" is not callable", callableName);
        handleException();
        Py_CLEAR(callable);
        Py_CLEAR(gcModule);
        return NULL;
    }

    PyObject *ret = PyObject_Call(callable, args, NULL);
    if (ret == NULL) {
        critical_log("[Python]: \"gc.%s\" raised error", callableName);
        handleException();
    }
    Py_CLEAR(callable);
    Py_CLEAR(gcModule);

    return ret;
}

void forceGarbageCollection() {
    trace_log("[Python]: Beginning python garbage collection");

    PyObject *gcCollectArgs = Py_BuildValue("()");
    PyObject *gcCollectRet = callGCMethod("collect", gcCollectArgs);
    Py_CLEAR(gcCollectArgs);
    if (gcCollectRet == NULL) {
        critical_log("[Python]: Failed python garbage collection: \"gc.collect\" raised error");
        handleException();
        return;
    }
    Py_CLEAR(gcCollectRet);
}

void dumpPythonObjects() {
    trace_log("[Python]: Beginning python object dump");

    PyObject *gcGetObjectsArgs = Py_BuildValue("()");
    PyObject *gcGetObjectsRet = callGCMethod("get_objects", gcGetObjectsArgs);
    Py_CLEAR(gcGetObjectsArgs);
    if (gcGetObjectsRet == NULL) return;

    if (!PyList_Check(gcGetObjectsRet)) {
        critical_log("[Python]: Failed python object dump: Expected return from \"gc.get_objects\" to be of type \"list\"");
        Py_CLEAR(gcGetObjectsRet);
        return;
    }

    Py_ssize_t listSize = PyList_Size(gcGetObjectsRet);
    for (Py_ssize_t i = 0; i < listSize; ++i) {
        PyObject *curItem = Py_XNewRef(PyList_GetItem(gcGetObjectsRet, i));
        if(curItem == NULL) {
            error_log("[Python]: Could not retrieve gc object at index \"%d\"", i);
            continue;
        }

        if (!isObjectInteresting(curItem)) {
            Py_CLEAR(curItem);
            continue;
        }

        PyObject *reprStr = PyObject_Repr(curItem);
        if (reprStr == NULL) {
            error_log("[Python]: Object Dump: Item \"%d\" raised error while determining repr string", i);
            handleException();
        } else {
            trace_log("[Python]: Object Dump: Item \"%d\" is \"%s\"; ref count was \"%d\"", i, PyUnicode_AsUTF8(reprStr), Py_REFCNT(curItem));

        }
        Py_CLEAR(reprStr);

        Py_CLEAR(curItem);
    }

    Py_CLEAR(gcGetObjectsRet);

    trace_log("[Python]: Ending python object dump");
}

// Please be aware that this returns a borrowed reference
PyObject *Py3d_GetTypeFromTuple(PyObject *tuple, Py_ssize_t index, PyTypeObject *type) {
    if (tuple == NULL || PyTuple_Check(tuple) != 1) {
        PyErr_SetString(PyExc_ValueError, "Invalid tuple supplied to Py3d_GetTypeFromTuple");
        return NULL;
    }

    PyObject *item = PyTuple_GetItem(tuple, index);
    if (item == NULL) return NULL;

    if (type == NULL || PyType_Check(type) != 1) return item;

    int typeCheck = PyObject_IsInstance(item, (PyObject *) type);
    if (typeCheck == -1) return NULL;

    if (typeCheck == 0) {
        PyErr_SetString(PyExc_ValueError, "Item at supplied index does not match expected type");
        return NULL;
    }

    return item;
}

struct Py3dGameObject *Py3d_GetComponentOwner(struct Py3dComponent *component) {
    PyObject *owner = Py3dComponent_GetOwner((struct Py3dComponent *) component, NULL);
    if (Py_IsNone(owner)) {
        Py_CLEAR(owner);
        return NULL;
    }

    if (!Py3dGameObject_Check(owner)) {
        Py_CLEAR(owner);
        critical_log("[Utility]: Py3dComponent has an owner that is not a GameObject");
        return NULL;
    }

    return (struct Py3dGameObject *) owner;
}

static PyObject* Py3d_GetParseData(
    PyObject *parseDataDict,
    PyTypeObject *type,
    const char *keyName,
    const char *componentName
) {
    if (parseDataDict == NULL || type == NULL || keyName == NULL || componentName == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Py3d_GetIntParseData received invalid params");
        return NULL;
    }

    PyObject *obj = PyDict_GetItemString(parseDataDict, keyName);
    if (obj == NULL) {
        PyErr_Format(PyExc_KeyError, "Parse data for \"%s\" must include a field called \"%s\"", componentName, keyName);
        return NULL;
    }
    if (!Py_IS_TYPE(obj, type)) {
        PyErr_Format(PyExc_KeyError, "Field with name \"%s\" must be of type \"%s\"", keyName, _PyType_Name(type));
        return NULL;
    }

    return obj;
}

int Py3d_GetBooleanParseData(PyObject *parseDataDict, const char *keyName, int *dst, const char *componentName) {
    if (dst == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Py3d_GetBooleanParseData received invalid params");
        return 0;
    }

    const PyObject *obj = Py3d_GetParseData(parseDataDict, &PyBool_Type, keyName, componentName);
    if (obj == NULL) return 0;

    *dst = Py_IsTrue(obj);
    return 1;
}

int Py3d_GetIntParseData(PyObject *parseDataDict, const char *keyName, int *dst, const char *componentName) {
    if (dst == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Py3d_GetIntParseData received invalid params");
        return 0;
    }

    PyObject *obj = Py3d_GetParseData(parseDataDict, &PyLong_Type, keyName, componentName);
    if (obj == NULL) return 0;

    const int result = (int) PyLong_AsLong(obj);
    if (PyErr_Occurred() != NULL) {
        return 0;
    }

    (*dst) = result;

    return 1;
}

int Py3d_GetFloatParseData(PyObject *parseDataDict, const char *keyName, float *dst, const char *componentName) {
    if (dst == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Py3d_GetIntParseData received invalid params");
        return 0;
    }

    PyObject *obj = Py3d_GetParseData(parseDataDict, &PyFloat_Type, keyName, componentName);
    if (obj == NULL) return 0;

    const float result = (float) PyFloat_AsDouble(obj);
    if (PyErr_Occurred()) return 0;

    (*dst) = result;

    return 1;
}

int Py3d_GetVector3ParseData(PyObject *parseDataDict, const char *keyName, float dst[3], const char *componentName) {
    if (dst == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Py3d_GetIntParseData received invalid params");
        return 0;
    }

    PyObject *obj = Py3d_GetParseData(parseDataDict, &Py3dVector3_Type, keyName, componentName);
    if (obj == NULL) return 0;

    Py3dVector3_AsFloatArray((struct Py3dVector3 *) obj, dst);

    return 1;
}