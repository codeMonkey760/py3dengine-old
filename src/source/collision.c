#include <ode/ode.h>

#include "collision.h"

int initCollisionEngine() {
    return dInitODE2(0);
}

void handleCollisions() {
    ;
}

void finalizeCollisionEngine() {
    dCloseODE();
}
