#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdbool.h>

#include "logger.h"
#include "python/python_wrapper.h"
#include "python/py3denginemodule.h"
#include "python/py3dmathmodule.h"
#include "python/py3dloggermodule.h"
#include "python/python_util.h"

static void addResourcesPath() {
    PyObject *sysMod = PyImport_ImportModule("sys");
    if (sysMod == NULL) {
        critical_log("[Python]: Could not import \"sys\" module");
        handleException();
        return;
    }

    PyObject *path = PyObject_GetAttrString(sysMod, "path");
    if (path == NULL) {
        critical_log("[Python]: Could not get reference to \"sys.path\"");
        handleException();
        Py_CLEAR(sysMod);
        return;
    }
    if (PyList_Check(path) != 1) {
        critical_log("[Python]: \"sys.path\" isn't a list for some reason");
        Py_CLEAR(path);
        Py_CLEAR(sysMod);
        return;
    }

    char buffer[256];
    memset(buffer, 0, sizeof(char) * 256);
    getcwd(buffer, 255);

    // TODO: this probably won't work on windows
    PyObject *cwdObj = PyUnicode_FromString(buffer);
    PyObject *resObj = PyUnicode_FromString("/resources");
    PyObject *fullPath = PyNumber_Add(cwdObj, resObj);
    Py_CLEAR(cwdObj);
    Py_CLEAR(resObj);


    if (PyList_Append(path, fullPath) != 0) {
        critical_log("[Python]: Modifying the import path failed");
    }

    Py_CLEAR(fullPath);
    Py_CLEAR(path);
    Py_CLEAR(sysMod);
}

bool initializePython(int argc, char **argv) {
    if (!appendPy3dEngineModule()) return false;
    if (!appendPy3dMathModule()) return false;
    if (!appendPy3dLoggerModule()) return false;

    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    PyStatus status = PyConfig_SetBytesArgv(&config, argc, argv);
    if (PyStatus_Exception(status)) {
        handleException();

        return false;
    }

    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status)) {
        PyConfig_Clear(&config);
        handleException();

        return false;
    }

    PyConfig_Clear(&config);

    if (!importPy3dEngineModule()) return false;
    if (!importPy3dMathModule()) return false;
    if (!importPy3dLoggerModule()) return false;

    if (!initPy3dEngineObjects()) return false;
    if (!initPy3dMathObjects()) return false;

    addResourcesPath();

    trace_log(
        "[Python]: Initialization successful: %s\n"
        "Version: %s\n"
        "Platform: %s",
        Py_IsInitialized() != 0 ? "TRUE" : "FALSE",
        Py_GetVersion(),
        Py_GetPlatform()
    );

    return true;
}

void finalizePython() {
    finalizePy3dMathModule();
    finalizePy3dEngineModule();

    Py_Finalize();
}