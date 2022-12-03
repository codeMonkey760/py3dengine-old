#ifndef PY3DENGINE_WINDOW_H
#define PY3DENGINE_WINDOW_H

#include <stdbool.h>

#include <GLFW/glfw3.h>

struct Window {
    GLFWwindow *window;
    int width;
    int height;
    bool full_screen;
};

extern void allocWindow(struct Window **windowPtr, int width, int height, bool fullscreen);
extern void deleteWindow(struct Window **windowPtr);

extern bool windowShouldClose(struct Window *window);
extern void swapBuffers(struct Window *window);
extern void pollEvents(struct Window *window);

#endif
