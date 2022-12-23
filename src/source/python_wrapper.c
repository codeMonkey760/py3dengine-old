#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "logger.h"
#include "python_wrapper.h"

static void handleInitializationException() {
    PyObject *type = NULL, *value = NULL, *traceback = NULL;
    PyErr_Fetch(&type, &value, &traceback);

    critical_log(
        "[Python]: Initialization yielded an error:\n"
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

void initializePython() {
    PyStatus status;

    PyConfig config;
    PyConfig_InitIsolatedConfig(&config);

    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status)) {
        PyConfig_Clear(&config);
        handleInitializationException();

        exit(1);
    }

    PyConfig_Clear(&config);

    trace_log(
        "[Python]: Initialization successful: %s\n"
        "Version: %s\n"
        "Platform: %s",
        Py_IsInitialized() != 0 ? "TRUE" : "FALSE",
        Py_GetVersion(),
        Py_GetPlatform()
    );
}

void finalizePython() {
    Py_Finalize();
}