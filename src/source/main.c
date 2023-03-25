#include <ode/ode.h>

#include "logger.h"
#include "engine.h"

int main(int argc, char **argv) {
    dInitODE2(0);
    dCloseODE();

    initLogger();

    initializeEngine(argc, argv);
    runEngine();
    finalizeEngine();

    return 0;
}
