#include "logger.h"
#include "engine.h"

int main(int argc, char **argv) {
    initLogger();

    initializeEngine(argc, argv);
    runEngine();
    finalizeEngine();

    return 0;
}
