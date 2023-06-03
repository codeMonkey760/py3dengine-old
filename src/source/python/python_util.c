#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "logger.h"
#include "python/python_util.h"
#include "python/py3dgameobject.h"
#include "python/py3dcomponent.h"
#include "python/py3dcollisionevent.h"
#include "python/py3dcontactpoint.h"
#include "python/py3drenderingcontext.h"
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