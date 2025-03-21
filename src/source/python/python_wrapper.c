#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdbool.h>

#include "logger.h"
#include "custom_string.h"
#include "custom_path.h"
#include "python/python_wrapper.h"
#include "python/py3denginemodule.h"
#include "python/py3dmathmodule.h"
#include "python/py3dloggermodule.h"
#include "python/py3dinput.h"
#include "python/python_util.h"

void appendImportPath(const char *relPath) {
    if (relPath == NULL) return;

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

    struct String *fullPath = NULL;
    createAbsolutePath(&fullPath, relPath);
    PyObject *fullPathObj = PyUnicode_FromString(getChars(fullPath));
    deleteString(&fullPath);

    int containsRet = PySequence_Contains(path, fullPathObj);
    if (containsRet == -1) {
        handleException();
    } else if (containsRet == 0) {
        if (PyList_Append(path, fullPathObj) != 0) {
            critical_log("[Python]: Modifying the import path failed");
        }
    }

    Py_CLEAR(fullPathObj);
    Py_CLEAR(path);
    Py_CLEAR(sysMod);
}

bool initializePython(int argc, char **argv) {
    if (!appendPy3dEngineModule()) return false;
    if (!appendPy3dMathModule()) return false;
    if (!appendPy3dLoggerModule()) return false;
    if (!appendPy3dInputModule()) return false;

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
    // input module does not need an import

    if (!initPy3dEngineObjects()) return false;
    if (!initPy3dMathObjects()) return false;
    // logger module does not need init
    // input module does not need init

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
    // input module does not need finalization
    // logger module does not need finalization
    finalizePy3dMathModule();
    finalizePy3dEngineModule();

    Py_Finalize();
}