#ifndef PY3DENGINE_BASE_RESOURCE_H
#define PY3DENGINE_BASE_RESOURCE_H

struct String;

struct BaseResource {
    unsigned int _type;
    struct String *_typeName;
    struct String *_name;
    void (*delete)(struct BaseResource **);
};

extern void initializeBaseResource(struct BaseResource *resource);
extern void finalizeBaseResource(struct BaseResource *resource);

extern struct String *getResourceName(struct BaseResource *resource);
extern void setResourceName(struct BaseResource *resource, const char *newName);

extern struct String *getResourceTypeName(struct BaseResource *resource);

#endif
