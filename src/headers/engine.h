#ifndef PY3DENGINE_ENGINE_H
#define PY3DENGINE_ENGINE_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <GLFW/glfw3.h>

struct Py3dScene;

extern GLFWwindow *glfwWindow;

extern void initializeEngine(int argc, char **argv);
extern void runEngine();
extern void finalizeEngine();
extern void getRenderingTargetDimensions(int *width, int *height);
extern struct Py3dScene *loadScene(const char *scenePath);
extern PyObject *activateScene(const char *sceneName);
extern PyObject *unloadScene(const char *sceneName);
extern void markWindowShouldClose();
extern int getCursorMode();
extern void setCursorMode(int newMode);

#endif
