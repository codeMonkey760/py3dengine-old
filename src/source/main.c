#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include "logger.h"
#include "engine.h"

int screenWidth = 800;
int screenHeight = 600;

static void error_callback(int code, const char* description) {
    error_log("%s 0x%x %s\n", "GLFW error code", code, description);
}

int main() {
    initLogger();

    glfwSetErrorCallback(error_callback);

    trace_log("Initializing GLFW");
    if (!glfwInit()) {
        return 1;
    }
    trace_log("GLFW successfully initialized");

    trace_log("Creating GLFW window");
    GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "Py3DEngine", NULL, NULL);
    if (!window) {
        return 1;
    }
    trace_log("Successfully created a window");

    trace_log("Creating OpenGL context and binding it to new window");
    glfwMakeContextCurrent(window);

    trace_log("Loading GLAD");
    int version = gladLoadGL(glfwGetProcAddress);
    trace_log("GL Version: %d.%d", GLAD_VERSION_MAJOR(version),GLAD_VERSION_MINOR(version));

    glfwSwapInterval(1);
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);

    struct Engine *engine = NULL;
    allocEngine(&engine);
    initEngine(engine);

    float prev_ts, cur_ts = 0.0f;
    while(!glfwWindowShouldClose(window)) {
        prev_ts = cur_ts;
        cur_ts = (float) glfwGetTime();
        float dt = cur_ts - prev_ts;

        glClear(GL_COLOR_BUFFER_BIT);

        updateEngine(engine, dt);
        renderEngine(engine);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    deleteEngine(&engine);
    engine = NULL;

    glfwTerminate();

    return 0;
}
