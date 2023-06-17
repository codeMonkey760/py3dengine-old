#include "python/py3dinput.h"

#include "logger.h"
#include "engine.h"

int convertIntToGlfwKey(int key) {
    if (key >= 65 && key <= 93) return key;
    if (key >= 44 && key <= 57) return key;
    if (key >= 290 && key <= 314) return key;

    if (key == 32) return key;
    if (key == 39) return key;
    if (key == 59) return key;
    if (key == 61) return key;
    if (key == 161 || key == 162) return key;
    if (key >= 256 && key <= 269) return key;
    if (key >= 280 && key <= 284) return key;
    if (key >= 320 && key <= 336) return key;
    if (key >= 340 && key <= 348) return key;

    return GLFW_KEY_UNKNOWN;
}

static PyObject *queryKeyState(PyObject *args, int expected_state) {
    PyObject *keyObj = NULL;
    if (PyArg_ParseTuple(args, "O!", &PyLong_Type, &keyObj) == 0) return NULL;

    int key = convertIntToGlfwKey((int) PyLong_AsLong(keyObj));
    if (key == GLFW_KEY_UNKNOWN) {
        PyErr_SetString(PyExc_ValueError, "Cannot convert param 1 to GLFW Key");
        return NULL;
    }

    int status = glfwGetKey(glfwWindow, key);
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

static PyObject *Py3dInput_GetCursorPos(PyObject *self, PyObject *args, PyObject *kwds) {
    double x = 0.0, y = 0.0;

    glfwGetCursorPos(glfwWindow, &x, &y);

    return Py_BuildValue("(dd)", x, y);
}

static PyMethodDef Py3dInput_Methods[] = {
    {"is_key_pressed", (PyCFunction) Py3dInput_IsKeyPressed, METH_VARARGS, "Determine if a keyboard key is depressed"},
    {"is_key_released", (PyCFunction) Py3dInput_IsKeyReleased, METH_VARARGS, "Determine if a keyboard key is released"},
    {"get_cursor_pos", (PyCFunction) Py3dInput_GetCursorPos, METH_NOARGS, "Query the current position of the cursor in screen pixels"},
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