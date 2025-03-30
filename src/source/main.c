#include "logger.h"
#include "engine.h"

int main(int argc, char **argv) {
    initLogger();

    if (!initializeEngine(argc, argv)) {
        return 1;
    }
    runEngine();
    finalizeEngine();

    return 0;
}
