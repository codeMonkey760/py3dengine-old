#ifndef PY3DENGINE_COLLISION_H
#define PY3DENGINE_COLLISION_H

#include <ode/ode.h>

struct PhysicsSpace {
    dWorldID world;
    dSpaceID space;
    struct CollisionState *collisionState;
};

extern void allocPhysicsSpace(struct PhysicsSpace **spacePtr);
extern void initPhysicsSpace(struct PhysicsSpace *space);
extern void deallocPhysicsSpace(struct PhysicsSpace **spacePtr);
extern void handleCollisions(struct PhysicsSpace *space);
extern dBodyID createDynamicsBody(struct PhysicsSpace *space);
extern void destroyDynamicsBody(dBodyID body);
extern void addGeomToWorldSpace(struct PhysicsSpace *space, dGeomID newGeom);
extern void removeGeomFromWorldSpace(struct PhysicsSpace *space, dGeomID geom);

#endif
