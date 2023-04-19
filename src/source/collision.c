#include "collision.h"
#include "python/py3dcollider.h"
#include "logger.h"

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

    struct dContactGeom contacts[8];
    int num_contacts = dCollide(o1, o2, 8, contacts, sizeof(dContactGeom));
    if (num_contacts == 0) return;

    // fetch Py3dColliders from o1 and o2 (hint: dGeomGetData should return a struct Py3dCollider *)
    struct Py3dCollider *collider1, *collider2;
    collider1 = dGeomGetData(o1);
    collider2 = dGeomGetData(o2);
    if (collider1 == NULL || collider2 == NULL) {
        critical_log("[Collision]: Improperly configured Geom discovered");
        return;
    }

    // if o1 and o2 don't belong to the same Py3dCollider (a Py3dCollider should have only one ODE Geom so this is a
    //    sanity check)
    if (collider1 == collider2) {
        warning_log("[Collision]: Colliding Geoms belong to same py3dcollider, discarding collision");
        return;
    }

    // if o1 and o2's Py3dColliders aren't owned by the same GameObject (this CAN and is likely to happen)
    PyObject *owner1, *owner2;
    owner1 = Py3dComponent_GetOwner((struct Py3dComponent *) collider1, NULL);
    owner2 = Py3dComponent_GetOwner((struct Py3dComponent *) collider2, NULL);
    if (Py_IsNone(owner1) || Py_IsNone(owner2)) {
        warning_log("[Collision]: Collision detected with detached py3dcollider, discarding collision");
        Py_CLEAR(owner1);
        Py_CLEAR(owner2);
        return;
    }
    if (owner1 == owner2) {
        Py_CLEAR(owner1);
        Py_CLEAR(owner2);
        return;
    }
    Py_CLEAR(owner1);
    Py_CLEAR(owner2);

    // if o1 and o2's Py3dColliders have settings that allow them to collide (not sure this will happen immediately,
    //    might be future features)
    // TODO: this is temporary, since we're not doing collision handling yet let's only allow collisions between
    //    colliders that have "isTrigger" set to true since collision handling isn't required for that use case
    if (!collider1->isTrigger || !collider2->isTrigger) return;

    // TODO: finish this
    // create a Py3dCollision event object
    // call collide on o1 and o2's owner GameObject passing the Py3dCollision object
    // clean up
}

void handleCollisions() {
    dSpaceCollide(space, NULL, (dNearCallback *) nearCallback);
}

void addGeomToWorldSpace(dGeomID newGeom) {
    dSpaceAdd(space, newGeom);
}

void removeGeomFromWorldSpace(dGeomID geom) {
    dSpaceRemove(space, geom);
}

void finalizeCollisionEngine() {
    dSpaceDestroy(space);

    dCloseODE();
}
