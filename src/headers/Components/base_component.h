#ifndef PY3DENGINE_BASE_COMPONENT_H
#define PY3DENGINE_BASE_COMPONENT_H

#include "camera.h"
#include "custom_string.h"

struct GameObject;

struct BaseComponent {
    unsigned int _type;
    struct String *_typeName;
    struct String *_name;
    struct GameObject *_owner;
    void (*update)(struct BaseComponent *, float);
    void (*render)(struct BaseComponent *, struct Camera *);
    void (*delete)(struct BaseComponent **);
};

extern void initializeBaseComponent(struct BaseComponent *component);
extern void finalizeBaseComponent(struct BaseComponent *component);

extern struct String *getComponentName(struct BaseComponent *component);
extern void setComponentName(struct BaseComponent *component, const char *newName);

extern struct String *getComponentTypeName(struct BaseComponent *component);

extern struct GameObject *getComponentOwner(struct BaseComponent *component);

#endif
