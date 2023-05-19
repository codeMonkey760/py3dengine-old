#include "physics/collision.h"
#include "python/py3dcollider.h"
#include "python/py3dcontactpoint.h"
#include "python/py3dcollisionevent.h"
#include "python/py3dgameobject.h"
#include "logger.h"

dWorldID world = NULL;
dSpaceID space = NULL;

static struct Py3dCollider *getColliderFromGeom(dGeomID geom) {
    PyObject *obj = NULL;

    obj = dGeomGetData(geom);
    if (obj == NULL) {
        critical_log("[Collision]: Geom without owner discovered");
        return NULL;
    }

    if (!Py3dCollider_Check(obj)) {
        critical_log("[Collision]: Geom with non collider owner discovered");
        return NULL;
    }

    return (struct Py3dCollider *) Py_NewRef(obj);
}

static struct Py3dGameObject *getOwnerFromCollider(struct Py3dCollider *collider) {
    PyObject *obj = NULL;

    obj = Py3dComponent_GetOwner((struct Py3dComponent *) collider, NULL);
    if (obj == NULL) {
        warning_log("[Collision]: Collision detected with detached py3dcollider, discarding collision");
        return NULL;
    }

    if (!Py3dGameObject_Check(obj)) {
        critical_log("[Collision]: Discovered component with an owner that is not a Game Object!");
        return NULL;
    }

    return (struct Py3dGameObject *) Py_NewRef(obj);
}

static void passCollideMessage(
    struct Py3dGameObject *owner,
    struct Py3dCollider *ownedCollider,
    struct Py3dCollider *otherCollider,
    PyObject *contactsTuple
) {
    struct Py3dCollisionEvent *event = NULL;

    event = Py3dCollisionEvent_New();
    Py_CLEAR(event->collider1);
    event->collider1 = Py_NewRef(ownedCollider);
    Py_CLEAR(event->collider2);
    event->collider2 = Py_NewRef(otherCollider);
    Py_CLEAR(event->contactsTuple);
    event->contactsTuple = Py_NewRef(contactsTuple);
    Py3dGameObject_Collide((struct Py3dGameObject *) owner, event);

    Py_CLEAR(event);
}

int initCollisionEngine() {
    if (!dInitODE2(0)) return 0;

    world = dWorldCreate();

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
    struct Py3dCollider *collider1 = getColliderFromGeom(o1);
    struct Py3dCollider *collider2 = getColliderFromGeom(o2);
    if (collider1 == NULL || collider2 == NULL) {
        Py_CLEAR(collider1);
        Py_CLEAR(collider2);
        return;
    }

    // if o1 and o2 don't belong to the same Py3dCollider (a Py3dCollider should have only one ODE Geom so this is a
    //    sanity check)
    if (collider1 == collider2) {
        warning_log("[Collision]: Colliding Geoms belong to same owner, discarding collision");
        Py_CLEAR(collider1);
        Py_CLEAR(collider2);
        return;
    }

    struct Py3dGameObject *owner1 = getOwnerFromCollider(collider1);
    struct Py3dGameObject *owner2 = getOwnerFromCollider(collider2);
    if (owner1 == NULL || owner2 == NULL) {
        Py_CLEAR(collider1);
        Py_CLEAR(collider2);
        Py_CLEAR(owner1);
        Py_CLEAR(owner2);
        return;
    }

    // if o1 and o2's Py3dColliders aren't owned by the same GameObject (this CAN and is likely to happen)
    if (owner1 == owner2) {
        Py_CLEAR(collider1);
        Py_CLEAR(collider2);
        Py_CLEAR(owner1);
        Py_CLEAR(owner2);
        return;
    }

    // if o1 and o2's Py3dColliders have settings that allow them to collide (not sure this will happen immediately,
    //    might be future features)
    // TODO: this is temporary, since we're not doing collision handling yet let's only allow collisions between
    //    colliders that have "isTrigger" set to true since collision handling isn't required for that use case
    if (!collider1->isTrigger || !collider2->isTrigger) return;

    // create a tuple containing contact info
    PyObject *contactsTuple = PyTuple_New(num_contacts);
    for (Py_ssize_t i = 0; i < num_contacts; ++i) {
        PyTuple_SetItem(contactsTuple, i, (PyObject *) Py3dContactPoint_New(&contacts[i]));
    }

    // create a collision event for collider1 to collider2 and call collide on collider1's owner
    passCollideMessage(owner1, collider1, collider2, contactsTuple);

    // create a collision event for collider2 to collider1 and call collide on collider2's owner
    passCollideMessage(owner2, collider2, collider1, contactsTuple);

    // clean up
    Py_CLEAR(contactsTuple);
    Py_CLEAR(collider1);
    Py_CLEAR(collider2);
    Py_CLEAR(owner1);
    Py_CLEAR(owner2);
}

void handleCollisions() {
    dSpaceCollide(space, NULL, (dNearCallback *) nearCallback);
}

dBodyID createDynamicsBody() {
    return dBodyCreate(world);
}

void destroyDynamicsBody(dBodyID body) {
    dBodyDestroy(body);
    body = NULL;
}

void addGeomToWorldSpace(dGeomID newGeom) {
    dSpaceAdd(space, newGeom);
}

void removeGeomFromWorldSpace(dGeomID geom) {
    dSpaceRemove(space, geom);
}

void finalizeCollisionEngine() {
    dSpaceDestroy(space);
    space = NULL;

    dWorldDestroy(world);
    world = NULL;

    dCloseODE();
}
