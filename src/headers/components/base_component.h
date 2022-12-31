#ifndef PY3DENGINE_BASE_COMPONENT_H
#define PY3DENGINE_BASE_COMPONENT_H

#include <stdbool.h>
#include <json-c/json.h>

struct String;
struct GameObject;
struct RenderingContext;
struct ResourceManager;

struct BaseComponent {
    unsigned int _type;
    struct String *_typeName;
    struct String *_name;
    struct GameObject *_owner;
    void (*update)(struct BaseComponent *, float);
    void (*render)(struct BaseComponent *, struct RenderingContext *);
    void (*resize)(struct BaseComponent *, int, int);
    bool (*parse)(struct BaseComponent *, json_object *, struct ResourceManager *);
    void (*delete)(struct BaseComponent **);
};

extern void initializeBaseComponent(struct BaseComponent *component);
extern void finalizeBaseComponent(struct BaseComponent *component);

extern struct String *getComponentName(struct BaseComponent *component);
extern void setComponentName(struct BaseComponent *component, const char *newName);

extern struct String *getComponentTypeName(struct BaseComponent *component);

extern struct GameObject *getComponentOwner(struct BaseComponent *component);

#endif
