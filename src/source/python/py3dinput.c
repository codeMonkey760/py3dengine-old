#include "python/py3dinput.h"

#include "logger.h"

static PyObject *Py3dInput_IsKeyPressed(PyObject *self, PyObject *args, PyObject *kwds) {
    Py_RETURN_FALSE;
}

static PyObject *Py3dInput_IsKeyReleased(PyObject *self, PyObject *args, PyObject *kwds) {
    Py_RETURN_FALSE;
}

static PyMethodDef Py3dInput_Methods[] = {
    {"is_key_pressed", (PyCFunction) Py3dInput_IsKeyPressed, METH_VARARGS, "Determine if a keyboard key is depressed"},
    {"is_key_released", (PyCFunction) Py3dInput_IsKeyReleased, METH_VARARGS, "Determine if a keyboard key is released"},
    {NULL}
};

static PyModuleDef Py3dInput_ModuleDef = {
    PyModuleDef_HEAD_INIT,
    .m_name = "py3dinput",
    .m_doc = "Functions for querying device input",
    .m_size = -1,
    .m_methods = Py3dInput_Methods
};

PyMODINIT_FUNC
PyInit_Py3dInput() {
    PyObject *newModule = PyModule_Create(&Py3dInput_ModuleDef);
    if (newModule == NULL) {
        critical_log("%s", "[Python]: Failed to create \"py3dinput\" module");

        return NULL;
    }

    return newModule;
}

int appendPy3dInputModule() {
    if (PyImport_AppendInittab("py3dinput", PyInit_Py3dInput) == -1) {
        critical_log("%s", "[Python]: Failed to extend built-in modules table with \"py3dinput\" module");
        return 0;
    }

    return 1;
}