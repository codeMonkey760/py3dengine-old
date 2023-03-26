#include <ode/ode.h>

#include "collision.h"

dSpaceID space = NULL;

int initCollisionEngine() {
    if (!dInitODE2(0)) return 0;

    space = dSimpleSpaceCreate(0);
    if (space == NULL) return 0;
    //destroy geoms registered to this space when its destroyed
    dSpaceSetCleanup(space, 1);

    return 1;
}

static void nearCallback(void *data, dGeomID o1, dGeomID o2) {
    // TODO: right now only one space is used as a container, so collisions between spaces arent supported
    // but this could become an issue later
    if (dGeomIsSpace(o1) || dGeomIsSpace(o2)) return;

    struct dContactGeom contacts[64];
    int num_contacts = dCollide(o1, o2, 64, contacts, sizeof(dContactGeom));

    // TODO: finish this
    // fetch Py3dColliders from o1 and o2 (hint: dGeomGetData should return a struct Py3dCollider *)
    // create a Py3dCollision event object
    // if o1 and o2 don't belong to the same Py3dCollider (a Py3dCollider should have only one ODE Geom so this is a
    //    sanity check)
    // if o1 and o2's Py3dColliders aren't owned by the same GameObject (this CAN and is likely to happen)
    // if o1 and o2's Py3dColliders have settings that allow them to collide (not sure this will happen immediately,
    //    might be future features)
    // call collide on o1 and o2's owner GameObject passing the Py3dCollision object
    // clean up
}

void handleCollisions() {
    dSpaceCollide(space, NULL, (dNearCallback *) nearCallback);
}

void finalizeCollisionEngine() {
    dSpaceDestroy(space);

    dCloseODE();
}
