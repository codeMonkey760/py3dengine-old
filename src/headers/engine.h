#ifndef PY3DENGINE_ENGINE_H
#define PY3DENGINE_ENGINE_H

#include <stdbool.h>

#include "quad.h"
#include "camera.h"

struct Engine {
    float _elapsed_time;
    float _fps;
    float _mpf;
    float _time_since_last_report;
    bool _print_report;

    struct Quad *quad[2];
    struct Camera *camera;
};

extern void allocEngine(struct Engine **enginePtr);
extern void deleteEngine(struct Engine **enginePtr);

extern void initEngine(struct Engine *engine);
extern void updateEngine(struct Engine *engine, float dt);
extern void renderEngine(struct Engine *engine);

#endif
