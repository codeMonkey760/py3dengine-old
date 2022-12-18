#ifndef PY3DENGINE_ENGINE_H
#define PY3DENGINE_ENGINE_H

#include <stdbool.h>

#include <GLFW/glfw3.h>

#include "quad.h"
#include "camera.h"
#include "model.h"

struct Engine {
    float _elapsed_time;
    float _fps;
    float _mpf;
    float _time_since_last_report;
    bool _print_report;

    GLFWwindow *window;

    struct Quad *quad[2];
    struct Camera *camera;
    struct Model *cubeModel;
    struct Model *pyramidModel;
    struct Model *quadModel;
};

extern void allocEngine(struct Engine **enginePtr);
extern void deleteEngine(struct Engine **enginePtr);

extern void initEngine(struct Engine *engine);
extern void runEngine(struct Engine *engine);

#endif
