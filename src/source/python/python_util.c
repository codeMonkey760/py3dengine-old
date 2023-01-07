#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "logger.h"
#include "python/python_util.h"

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
