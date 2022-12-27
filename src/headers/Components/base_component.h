#ifndef PY3DENGINE_BASE_COMPONENT_H
#define PY3DENGINE_BASE_COMPONENT_H

#include "camera.h"
#include "custom_string.h"

struct BaseComponent {
    unsigned int _type;
    struct String *_name;
    void (*update)(struct BaseComponent *, float);
    void (*render)(struct BaseComponent *, struct Camera *);
    void (*delete)(struct BaseComponent **);
};

extern void finalizeBaseComponent(struct BaseComponent *component);

extern struct String *getComponentName(struct BaseComponent *component);
extern void setComponentName(struct BaseComponent *component, const char *newName);

#endif
