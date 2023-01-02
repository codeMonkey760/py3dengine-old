#ifndef PY3DENGINE_RESOURCE_MANAGER_H
#define PY3DENGINE_RESOURCE_MANAGER_H

struct BaseResource;

struct ResourceManager {
    struct ListNode *_root;
};

extern void allocResourceManager(struct ResourceManager **resourceManagerPtr);
extern void deleteResourceManager(struct ResourceManager **resourceManagerPtr);

extern void storeResource(struct ResourceManager *manager, struct BaseResource *resource);
extern struct BaseResource *getResource(struct ResourceManager *manager, const char *name);

#endif
