#ifndef PY3DENGINE_PY3DINPUT_H
#define PY3DENGINE_PY3DINPUT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <GLFW/glfw3.h>

extern int appendPy3dInputModule();
extern int importPy3dInputModule();

extern void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
extern void finalizeCallbackList();

#endif
