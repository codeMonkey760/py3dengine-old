#include "logger.h"
#include "engine.h"

int main() {
    initLogger();

    initializeEngine();
    runEngine();
    finalizeEngine();

    return 0;
}
