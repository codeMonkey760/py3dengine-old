#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "logger.h"
#include "python/python_util.h"

void handleException() {
    PyObject *type = NULL, *value = NULL, *traceback = NULL;
    PyErr_Fetch(&type, &value, &traceback);

    critical_log(
            "[Python]: Operation yielded an error:\n"
            "Type: %s\n"
            "Value: %s\n"
            "Traceback: %s",
            (type != NULL) ? PyUnicode_AsUTF8(type) : "NULL",
            (value != NULL) ? PyUnicode_AsUTF8(value): "NULL",
            (traceback != NULL) ? PyUnicode_AsUTF8(traceback): "NULL"
    );

    Py_XDECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);
}
