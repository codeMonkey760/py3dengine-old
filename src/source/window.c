#include <stdlib.h>

#include "window.h"

void allocWindow(struct Window **windowPtr, int width, int height, bool full_screen) {
    if (windowPtr == NULL || (*windowPtr) != NULL) return;

    struct Window *window = NULL;
    window = calloc(1, sizeof(struct Window));
    if (window == NULL) return;

    GLFWwindow *glfwWindow = glfwCreateWindow(width, height, "Py3DEngine", NULL, NULL);
    if (glfwWindow == NULL) {
        deleteWindow(&window);
        return;
    }

    window->window = glfwWindow;
    window->width = width;
    window->height = height;
    window->full_screen = false;

    glfwMakeContextCurrent(window->window);

    (*windowPtr) = window;
    window = NULL;
}

void deleteWindow(struct Window **windowPtr) {
    if (windowPtr == NULL || (*windowPtr) == NULL) return;

    struct Window *window = (*windowPtr);
    if (window->window != NULL) {
        glfwDestroyWindow(window->window);
        window->window = NULL;
    }

    free(window);
    window = NULL;
    (*windowPtr) = NULL;
}

// TODO: this has nothing to do with windows, this should go into a new class for managing the current opengl context
void setSwapInterval(int newSwapInterval) {
    glfwSwapInterval(newSwapInterval);
}

bool windowShouldClose(struct Window *window) {
    // this function is used as a conditional for while loops
    // returning false when there is no window could lead to infinite loop
    if (window == NULL || window->window == NULL) return true;

    return glfwWindowShouldClose(window->window) == GL_TRUE;
}

void swapBuffers(struct Window *window) {
    if (window == NULL || window->window == NULL) return;

    glfwSwapBuffers(window->window);
}

// TODO: this has nothing to do with windows, this should go into a new class for managing the current opengl context
void pollEvents(){
    glfwPollEvents();
}
