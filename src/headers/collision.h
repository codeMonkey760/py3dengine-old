#ifndef PY3DENGINE_COLLISION_H
#define PY3DENGINE_COLLISION_H

#include <ode/ode.h>

int initCollisionEngine();
void handleCollisions();
dBodyID createDynamicsBody();
void destroyDynamicsBody(dBodyID body);
void addGeomToWorldSpace(dGeomID newGeom);
void removeGeomFromWorldSpace(dGeomID geom);
void finalizeCollisionEngine();

#endif
