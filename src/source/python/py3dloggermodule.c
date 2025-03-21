#include "python/py3dloggermodule.h"
#include "logger.h"
#include "python/python_util.h"

static PyObject *Py3dLogger_Debug(PyObject *self, PyObject *args, PyObject *kwds) {
    const char *msg = NULL;
    if (PyArg_ParseTuple(args, "z", &msg) != 1) return NULL;

    if (msg != NULL) {
        debug_log("%s", msg);
    }

    Py_RETURN_NONE;
}

static PyObject *Py3dLogger_Trace(PyObject *self, PyObject *args, PyObject *kwds) {
    const char *msg = NULL;
    if (PyArg_ParseTuple(args, "z", &msg) != 1) return NULL;

    if (msg != NULL) {
        trace_log("%s", msg);
    }

    Py_RETURN_NONE;
}

static PyObject *Py3dLogger_Info(PyObject *self, PyObject *args, PyObject *kwds) {
    const char *msg = NULL;
    if (PyArg_ParseTuple(args, "z", &msg) != 1) return NULL;

    if (msg != NULL) {
        info_log("%s", msg);
    }

    Py_RETURN_NONE;
}

static PyObject *Py3dLogger_Warning(PyObject *self, PyObject *args, PyObject *kwds) {
    const char *msg = NULL;
    if (PyArg_ParseTuple(args, "z", &msg) != 1) return NULL;

    if (msg != NULL) {
        warning_log("%s", msg);
    }

    Py_RETURN_NONE;
}

static PyObject *Py3dLogger_Error(PyObject *self, PyObject *args, PyObject *kwds) {
    const char *msg = NULL;
    if (PyArg_ParseTuple(args, "z", &msg) != 1) return NULL;

    if (msg != NULL) {
        error_log("%s", msg);
    }

    Py_RETURN_NONE;
}

static PyObject *Py3dLogger_Critical(PyObject *self, PyObject *args, PyObject *kwds) {
    const char *msg = NULL;
    if (PyArg_ParseTuple(args, "z", &msg) != 1) return NULL;

    if (msg != NULL) {
        critical_log("%s", msg);
    }

    Py_RETURN_NONE;
}

PyMethodDef Py3dLogger_Methods[] = {
    {"debug", (PyCFunction) Py3dLogger_Debug, METH_VARARGS, "Print message to debug log channel"},
    {"trace", (PyCFunction) Py3dLogger_Trace, METH_VARARGS, "Print message to trace log channel"},
    {"info", (PyCFunction) Py3dLogger_Info, METH_VARARGS, "Print message to info log channel"},
    {"warning", (PyCFunction) Py3dLogger_Warning, METH_VARARGS, "Print message to warning log channel"},
    {"error", (PyCFunction) Py3dLogger_Error, METH_VARARGS, "Print message to error log channel"},
    {"critical", (PyCFunction) Py3dLogger_Critical, METH_VARARGS, "Print message to critical log channel"},
    {NULL}
};

static struct PyModuleDef Py3dLogger_ModuleDef = {
    PyModuleDef_HEAD_INIT,
    .m_name = "py3dengineEXT",
    .m_doc = "Functions for writing to the configured logs",
    .m_size = -1,
    .m_methods = Py3dLogger_Methods
};

static PyObject *module = NULL;

PyMODINIT_FUNC
PyInit_Py3dLogger(void) {
    PyObject *newModule = PyModule_Create(&Py3dLogger_ModuleDef);
    if (newModule == NULL) {
        critical_log("%s", "[Python]: Failed to create \"py3dlogger\" module");

        return NULL;
    }

    return newModule;
}

bool appendPy3dLoggerModule() {
    if (PyImport_AppendInittab("py3dlogger", PyInit_Py3dLogger) == -1) {
        critical_log("%s", "[Python]: Failed to extend built-in modules table with \"py3dlogger\" module");
        return false;
    }

    return true;
}

bool importPy3dLoggerModule() {
    module = PyImport_ImportModule("py3dlogger");
    if (module == NULL) {
        critical_log("%s", "[Python]: Could not import py3dlogger");
        handleException();
        return false;
    }

    return true;
}
