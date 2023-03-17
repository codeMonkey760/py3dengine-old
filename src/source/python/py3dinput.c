#include "python/py3dinput.h"
#include "python/python_util.h"

#include "logger.h"
#include "engine.h"

PyObject *callbackTable[GLFW_KEY_MENU+1][GLFW_REPEAT+1][64] = {NULL};

static int convertIntToGlfwKey(int key) {
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

void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    PyObject *callback = callbackTable[key][action][mods];
    if (callback == NULL) return;

    PyObject *ret = PyObject_CallNoArgs(callback);
    if (ret == NULL) {
        error_log("[Input]: Key callback threw exception when handling key:%d action:%d mods:%d", key, action, mods);
        handleException();
    }

    Py_CLEAR(ret);
}

void finalizeCallbackTable() {
    for (int i = 0; i < GLFW_KEY_MENU+1; ++i) {
        for (int j = 0; j < GLFW_REPEAT+1; ++j) {
            for (int k = 0; k < 64; ++k) {
                Py_CLEAR(callbackTable[i][j][k]);
            }
        }
    }
}

static PyObject *Py3dInput_IsKeyPressed(PyObject *self, PyObject *args, PyObject *kwds) {
    return queryKeyState(args, GLFW_PRESS);
}

static PyObject *Py3dInput_IsKeyReleased(PyObject *self, PyObject *args, PyObject *kwds) {
    return queryKeyState(args, GLFW_RELEASE);
}

static PyObject *Py3dInput_SetKeyCallback(PyObject *self, PyObject *args, PyObject *kwds) {
    PyObject *callback = NULL, *keyObj = NULL, *actionObj = NULL, *modsObj = NULL;

    PyArg_ParseTuple(args, "OO!O!O!", &callback, &PyLong_Type, &keyObj, &PyLong_Type, &actionObj, &PyLong_Type, &modsObj);
    if (!PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_ValueError, "Param 1 must be callable callback");
        return NULL;
    }

    int key = convertIntToGlfwKey((int) PyLong_AsLong(keyObj));
    if (key == GLFW_KEY_UNKNOWN) {
        PyErr_SetString(PyExc_ValueError, "Cannot convert param 2 to GLFW Key");
        return NULL;
    }

    int action = (int) PyLong_AsLong(actionObj);
    if (action != GLFW_PRESS && action != GLFW_RELEASE && action != GLFW_REPEAT) {
        PyErr_SetString(PyExc_ValueError, "Cannot convert param 3 to GLFW Action");
        return NULL;
    }

    int mods = (int) PyLong_AsLong(modsObj);
    if (mods > 63) {
        PyErr_SetString(PyExc_ValueError, "Cannot convert param 4 to GLFW Action");
        return NULL;
    }

    Py_INCREF(callback);
    callbackTable[key][action][mods] = callback;

    Py_RETURN_NONE;
}

static PyObject *Py3dInput_GetCursorPos(PyObject *self, PyObject *args, PyObject *kwds) {
    double x = 0.0, y = 0.0;

    glfwGetCursorPos(glfwWindow, &x, &y);

    return Py_BuildValue("(dd)", x, y);
}

static PyObject *Py3dInput_SetCursorMode(PyObject *self, PyObject *args, PyObject *kwds) {
    char *newMode = NULL;

    if (PyArg_ParseTuple(args, "s", &newMode) != 1) return NULL;

    int glfwCursorMode;
    if (strcmp(newMode, "NORMAL") == 0) {
        glfwCursorMode = GLFW_CURSOR_NORMAL;
    } else if (strcmp(newMode, "DISABLED") == 0) {
        glfwCursorMode = GLFW_CURSOR_DISABLED;
    } else if (strcmp(newMode, "HIDDEN") == 0) {
        glfwCursorMode = GLFW_CURSOR_HIDDEN;
    } else {
        PyErr_SetString(PyExc_ValueError, "Unrecognized cursor mode");
        return NULL;
    }

    glfwSetInputMode(glfwWindow, GLFW_CURSOR, glfwCursorMode);

    Py_RETURN_NONE;
}

static PyMethodDef Py3dInput_Methods[] = {
    {"is_key_pressed", (PyCFunction) Py3dInput_IsKeyPressed, METH_VARARGS, "Determine if a keyboard key is depressed"},
    {"is_key_released", (PyCFunction) Py3dInput_IsKeyReleased, METH_VARARGS, "Determine if a keyboard key is released"},
    {"set_key_callback", (PyCFunction) Py3dInput_SetKeyCallback, METH_VARARGS, "Register a callback to be executed when a keyboard event happens"},
    {"get_cursor_pos", (PyCFunction) Py3dInput_GetCursorPos, METH_NOARGS, "Query the current position of the cursor in screen pixels"},
    {"set_cursor_mode", (PyCFunction) Py3dInput_SetCursorMode, METH_VARARGS, "Set the cursor mode"},
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