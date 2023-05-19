#ifndef PY3DENGINE_COLLISION_LIST_H
#define PY3DENGINE_COLLISION_LIST_H

struct Py3dCollider;
struct CollisionListEntry;

struct CollisionList {
    struct CollisionListEntry *head;
};

extern void allocCollisionTable(struct CollisionList **tablePtr);
extern void deallocCollisionTable(struct CollisionList **tablePtr);
extern void addCollisionToTable(struct CollisionList *table, struct Py3dCollider *key, struct Py3dCollider *value);

#endif
