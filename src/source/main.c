#include "logger.h"
#include "engine.h"

int main() {
    initLogger();

    struct Engine *engine = NULL;
    allocEngine(&engine);
    initEngine(engine);
    runEngine(engine);
    deleteEngine(&engine);
    engine = NULL;

    return 0;
}
