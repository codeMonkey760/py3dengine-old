#include <stdlib.h>

#include "window.h"

void allocWindow(struct Window **windowPtr, int width, int height, bool full_screen) {
    if (windowPtr == NULL || (*windowPtr) != NULL) return;

    struct Window *window = NULL;
    window = calloc(1, sizeof(struct Window));
    if (window == NULL) return;

    window->window = glfwCreateWindow(width, height, "Py3DEngine", NULL, NULL);
    if (window->window == NULL) {
        deleteWindow(&window);
        return;
    }

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

