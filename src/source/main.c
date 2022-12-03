#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include "logger.h"
#include "engine.h"

int main() {
    initLogger();

    trace_log("Initializing GLFW");
    if (!glfwInit()) {
        return 1;
    }
    trace_log("GLFW successfully initialized");

    struct Engine *engine = NULL;
    allocEngine(&engine);

    trace_log("Loading GLAD");
    int version = gladLoadGL(glfwGetProcAddress);
    trace_log("GL Version: %d.%d", GLAD_VERSION_MAJOR(version),GLAD_VERSION_MINOR(version));

    initEngine(engine);

    runEngine(engine);
    deleteEngine(&engine);
    engine = NULL;

    glfwTerminate();

    return 0;
}
