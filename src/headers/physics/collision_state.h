#ifndef PY3DENGINE_COLLISION_STATE_H
#define PY3DENGINE_COLLISION_STATE_H

struct Py3dCollider;
struct CollisionStateEntry;

struct CollisionState {
    struct CollisionStateEntry *head;
};

struct CollisionStateDiffEntry {
    struct Py3dCollider *c1;
    struct Py3dCollider *c2;
    int isAddition;
    struct CollisionStateDiffEntry *next;
};

struct CollisionStateDiff {
    struct CollisionStateDiffEntry *head;
};

extern void allocCollisionState(struct CollisionState **statePtr);
extern void deallocCollisionState(struct CollisionState **statePtr);
extern void addCollisionToState(struct CollisionState *state, struct Py3dCollider *key, struct Py3dCollider *value);

extern void allocCollisionStateDiff(struct CollisionStateDiff **diffPtr);
extern void deallocCollisionStateDiff(struct CollisionStateDiff **diffPtr);
extern void calculateCollisionStateDiff(struct CollisionStateDiff *diff, struct CollisionState *state1, struct CollisionState *state2);

#endif
