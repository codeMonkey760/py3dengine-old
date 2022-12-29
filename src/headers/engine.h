#ifndef PY3DENGINE_ENGINE_H
#define PY3DENGINE_ENGINE_H

#include <stdbool.h>

#include <GLFW/glfw3.h>

#include "resource_manager.h"
#include "model.h"
#include "shader.h"

struct GameObject;

struct Engine {
    float _elapsed_time;
    float _fps;
    float _mpf;
    float _time_since_last_report;
    bool _print_report;

    GLFWwindow *window;

    struct ResourceManager *resourceManager;

    struct GameObject *root;
    struct GameObject *activeCamera;
};

extern void allocEngine(struct Engine **enginePtr);
extern void deleteEngine(struct Engine **enginePtr);

extern void initEngine(struct Engine *engine);
extern void runEngine(struct Engine *engine);

#endif
