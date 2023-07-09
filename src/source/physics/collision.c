#include "physics/collision.h"
#include "physics/collision_state.h"
#include "python/py3dcollider.h"
#include "python/py3dcontactpoint.h"
#include "python/py3dcollisionevent.h"
#include "python/py3dgameobject.h"
#include "logger.h"

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

    return (struct Py3dGameObject *) obj;
}

static struct Py3dCollisionEvent *createCollisionEvent(struct Py3dCollider *c1, struct Py3dCollider *c2) {
    struct Py3dCollisionEvent *event = Py3dCollisionEvent_New();

    Py_CLEAR(event->collider1);
    event->collider1 = Py_NewRef(c1);
    Py_CLEAR(event->collider2);
    event->collider2 = Py_NewRef(c2);

    return event;
}

static void passColliderEnterMessage(
    struct Py3dGameObject *owner,
    struct Py3dCollider *ownedCollider,
    struct Py3dCollider *otherCollider
) {
    struct Py3dCollisionEvent *event = createCollisionEvent(ownedCollider, otherCollider);

    Py3dGameObject_ColliderEnter((struct Py3dGameObject *) owner, event);

    Py_CLEAR(event);
}

static void passColliderExitMessage(
    struct Py3dGameObject *owner,
    struct Py3dCollider *ownedCollider,
    struct Py3dCollider *otherCollider
) {
    struct Py3dCollisionEvent *event = createCollisionEvent(ownedCollider, otherCollider);

    Py3dGameObject_ColliderExit((struct Py3dGameObject *) owner, event);

    Py_CLEAR(event);
}

static void passCollideMessage(
    struct Py3dGameObject *owner,
    struct Py3dCollider *ownedCollider,
    struct Py3dCollider *otherCollider,
    PyObject *contactsTuple
) {
    struct Py3dCollisionEvent *event = createCollisionEvent(ownedCollider, otherCollider);

    Py_CLEAR(event->contactsTuple);
    event->contactsTuple = Py_NewRef(contactsTuple);
    Py3dGameObject_Collide((struct Py3dGameObject *) owner, event);

    Py_CLEAR(event);
}

void allocPhysicsSpace(struct PhysicsSpace **spacePtr) {
    if (spacePtr == NULL || (*spacePtr) != NULL) return;

    struct PhysicsSpace *newSpace = calloc(1, sizeof(struct PhysicsSpace));
    if (newSpace == NULL) return;

    newSpace->collisionState = NULL;
    newSpace->space = NULL;
    newSpace->world = NULL;

    (*spacePtr) = newSpace;
    newSpace = NULL;
}

void initPhysicsSpace(struct PhysicsSpace *space) {
    if (space == NULL) return;

    space->world = dWorldCreate();
    if (space->world == NULL) return;

    space->space = dSimpleSpaceCreate(0);
    if (space->space == NULL) return;
    //destroy geoms registered to this space when its destroyed
    dSpaceSetCleanup(space->space, 1);

    allocCollisionState(&space->collisionState);
}

void deallocPhysicsSpace(struct PhysicsSpace **spacePtr) {
    if (spacePtr == NULL || (*spacePtr == NULL)) return;

    struct PhysicsSpace *space = (*spacePtr);

    deallocCollisionState(&space->collisionState);

    dSpaceDestroy(space->space);
    space->space = NULL;

    dWorldDestroy(space->world);
    space->world = NULL;

    free(space);
    space = NULL;
    (*spacePtr) = NULL;
}

static void nearCallback(void *data, dGeomID o1, dGeomID o2) {
    if (data == NULL) return;

    struct PhysicsSpace *space = (struct PhysicsSpace *) data;

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
    if (!collider1->isTrigger || !collider2->isTrigger) {
        Py_CLEAR(collider1);
        Py_CLEAR(collider2);
        Py_CLEAR(owner1);
        Py_CLEAR(owner2);
        return;
    }

    // add Collision to current collision state
    addCollisionToState(space->collisionState, collider1, collider2);
    addCollisionToState(space->collisionState, collider2, collider1);

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

static void handleCollisionEvents(struct CollisionStateDiff *diff) {
    if (diff == NULL) return;

    struct CollisionStateDiffEntry *curEntry = diff->head;
    while (curEntry != NULL) {
        struct Py3dGameObject *owner = getOwnerFromCollider(curEntry->c1);
        if (owner == NULL) continue;

        if (curEntry->isAddition == 0) {
            passColliderExitMessage(owner, curEntry->c1, curEntry->c2);
        } else if (curEntry->isAddition == 1) {
            passColliderEnterMessage(owner, curEntry->c1, curEntry->c2);
        } else {
            warning_log("[Collision]: Bad collision event enum detected");
        }

        Py_CLEAR(owner);

        curEntry = curEntry->next;
    }
}

void handleCollisions(struct PhysicsSpace *space) {
    if (space == NULL) return;

    struct CollisionState *prevState = space->collisionState;
    space->collisionState = NULL;
    allocCollisionState(&space->collisionState);

    dSpaceCollide(space->space, space, (dNearCallback *) nearCallback);

    struct CollisionStateDiff *diff = NULL;
    allocCollisionStateDiff(&diff);
    calculateCollisionStateDiff(diff, prevState, space->collisionState);

    deallocCollisionState(&prevState);
    handleCollisionEvents(diff);

    deallocCollisionStateDiff(&diff);
}

dBodyID createDynamicsBody(struct PhysicsSpace *space) {
    if (space == NULL) return NULL;

    return dBodyCreate(space->world);
}

void destroyDynamicsBody(dBodyID body) {
    if (body == NULL) return;

    dBodyDestroy(body);
    body = NULL;
}

void addGeomToWorldSpace(struct PhysicsSpace *space, dGeomID newGeom) {
    dSpaceAdd(space->space, newGeom);
}

void removeGeomFromWorldSpace(struct PhysicsSpace *space, dGeomID geom) {
    dSpaceRemove(space->space, geom);
}

