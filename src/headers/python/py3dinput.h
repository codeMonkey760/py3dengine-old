#ifndef PY3DENGINE_PY3DINPUT_H
#define PY3DENGINE_PY3DINPUT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <GLFW/glfw3.h>

extern int appendPy3dInputModule();

extern int convertIntToGlfwKey(int key);

#endif
