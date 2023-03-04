#include <GLFW/glfw3.h>
#include "python/py3dinput.h"

#include "logger.h"
#include "engine.h"

static PyObject *queryKeyState(PyObject *args, int expected_state) {
    PyObject *keyObj = NULL;
    if (PyArg_ParseTuple(args, "O!", &PyUnicode_Type, &keyObj) == 0) return NULL;

    if (PyUnicode_GetLength(keyObj) != 1) {
        PyErr_SetString(PyExc_ValueError, "Expected string with length of 1");
        return NULL;
    }

    char keycode = PyUnicode_AsUTF8(keyObj)[0];
    keycode = (char) toupper(keycode);

    int status = glfwGetKey(glfwWindow, keycode);
    if (status == expected_state) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

static PyObject *Py3dInput_IsKeyPressed(PyObject *self, PyObject *args, PyObject *kwds) {
    return queryKeyState(args, GLFW_PRESS);
}

static PyObject *Py3dInput_IsKeyReleased(PyObject *self, PyObject *args, PyObject *kwds) {
    return queryKeyState(args, GLFW_RELEASE);
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