#include "python/py3dmathmodule.h"
#include "logger.h"
#include "python/python_util.h"

static struct PyModuleDef py3dmathModuleDef = {
        PyModuleDef_HEAD_INIT,
        .m_name = "py3dmath",
        .m_doc = "Contains 3d math related objects and functions",
        .m_size = -1,
};

static PyObject *module = NULL;

PyMODINIT_FUNC
PyInit_py3dMath(void) {
    PyObject *newModule = PyModule_Create(&py3dmathModuleDef);
    if (newModule == NULL) {
        critical_log("%s", "[Python]: Failed to create \"py3dmath\" module");

        return NULL;
    }

    return newModule;
}

bool appendPy3dMathModule() {
    if (PyImport_AppendInittab("py3dmath", PyInit_py3dMath) == -1) {
        critical_log("%s", "[Python]: Failed to extend built-in modules table with py3dmath module");
        return false;
    }

    return true;
}

bool importPy3dMathModule() {
    module = PyImport_ImportModule("py3dmath");
    if (module == NULL) {
        critical_log("%s", "[Python]: Could not import py3dmath");
        handleException();
        return false;
    }

    return true;
}

bool initPy3dMathObjects() {
    return true;
}

void finalizePy3dMathModule() {

}