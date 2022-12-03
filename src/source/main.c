#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "logger.h"
#include "shader.h"
#include "quadmodel.h"
#include "quad.h"
#include "camera.h"

int screenWidth = 800;
int screenHeight = 600;

static void error_callback(int code, const char* description) {
    error_log("%s 0x%x %s\n", "GLFW error code", code, description);
}

static void update(float dt) {
    static int frame_count = 0;
    static float since_last_report = 0.0f;

    since_last_report += dt;
    frame_count += 1;

    if (since_last_report >= 1.0f) {
        float mpf = (since_last_report / (float) frame_count) * 1000.0f;
        printf("FPS: %d MPF: %f\n", frame_count, mpf);

        frame_count = 0;
        while (since_last_report >= 1.0f) {
            since_last_report -= 1.0f;
        }
    }
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
    printf("GL %d.%d\n",GLAD_VERSION_MAJOR(version),GLAD_VERSION_MINOR(version));

    initShader();
    initQuadModel();
    float quadPosW[3] = {-2.0f, 0.0f, 0.0f};
    struct Quad *quad1 = NULL, *quad2 = NULL;
    allocQuad(&quad1);
    setPosW(quad1, quadPosW);
    allocQuad(&quad2);
    quadPosW[0] = 2.0f;
    setPosW(quad2, quadPosW);
    struct Camera *camera = NULL;
    allocCamera(&camera);

    glfwSwapInterval(1);
    glClearColor(0.25f, 0.25f, 0.75f, 1.0f);

    float prev_ts, cur_ts = 0.0f;
    while(!glfwWindowShouldClose(window)) {
        prev_ts = cur_ts;
        cur_ts = (float) glfwGetTime();
        float dt = cur_ts - prev_ts;

        glClear(GL_COLOR_BUFFER_BIT);

        update(dt);
        updateCamera(camera, dt);

        updateQuad(quad1, dt);
        updateQuad(quad2, dt);

        renderQuad(quad1, camera);
        renderQuad(quad2, camera);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    deleteCamera(&camera);
    deleteQuad(&quad2);
    deleteQuad(&quad1);
    deleteQuadModel();
    deleteShader();

    glfwTerminate();

    return 0;
}
