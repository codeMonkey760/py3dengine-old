#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdbool.h>

#include "logger.h"
#include "python/python_wrapper.h"
#include "python/py3denginemodule.h"
#include "python/py3dmathmodule.h"
#include "python/py3dloggermodule.h"
#include "python/python_util.h"

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