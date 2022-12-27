#ifndef PY3DENGINE_BASE_COMPONENT_H
#define PY3DENGINE_BASE_COMPONENT_H

#include "camera.h"

struct BaseComponent {
    unsigned int _type;
    void (*update)(void *, float);
    void (*render)(void *, struct Camera *);
    void (*delete)(void **);
};

#endif
