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
    ;
}

void handleCollisions() {
    dSpaceCollide(space, NULL, (dNearCallback *) nearCallback);
}

void finalizeCollisionEngine() {
    dSpaceDestroy(space);

    dCloseODE();
}
