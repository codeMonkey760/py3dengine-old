#ifndef PY3DENGINE_ENGINE_H
#define PY3DENGINE_ENGINE_H

#include <GLFW/glfw3.h>

extern GLFWwindow *glfwWindow;

extern void initializeEngine(int argc, char **argv);
extern void runEngine();
extern void finalizeEngine();
extern void getRenderingTargetDimensions(int *width, int *height);

#endif
