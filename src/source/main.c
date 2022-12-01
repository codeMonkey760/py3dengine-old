#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "util.h"
#include "shader.h"

GLuint vao = -1;
GLuint vbo = -1;
GLuint posShaderIndex = 0;

int screenWidth = 800;
int screenHeight = 600;
float diffuseColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

float pMtx[16] = {0.0f};

static void error_callback(int code, const char* description) {
    fprintf(stderr, "[%s]: %s 0x%x %s\n", LOG_ERROR, "GLFW error code", code, description);
}

static void level_log(const char *level, const char *message) {
    printf("[%s]: %s\n", level, message);
}

static void info_log(const char *message) {
    level_log(LOG_INFO, message);
}

static void trace_log(const char *message) {
    level_log(LOG_TRACE, message);
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

static void initQuadModel() {
    float max = 0.8f;
    float min = -0.8f;

    float vertexData[6][3] = {
            {min, min, 0.0f},
            {min, max, 0.0f},
            {max, max, 0.0f},

            {max, max, 0.0f},
            {max, min, 0.0f},
            {min, min, 0.0f},
    };
    const int bufferSizeInElements = 6;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, bufferSizeInElements * 96, vertexData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(posShaderIndex);
    glVertexAttribPointer(posShaderIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
}

static void deleteQuadModel() {
    glDeleteBuffers(1, &vbo);
    vbo = -1;
    glDeleteVertexArrays(1, &vao);
    vao = -1;
}

static void updateQuad(float dt) {
    const static float cycle_rate = M_PI_2;
    static float theta = 0.0f;

    theta += dt * cycle_rate;
    while (theta >= M_TWO_PI) {
        theta -= M_TWO_PI;
    }

    diffuseColor[0] = (cosf(theta) * 0.5f) + 0.5f;
    diffuseColor[1] = 0.0f;
    diffuseColor[2] = (sinf(theta) * 0.5f) + 0.5f;
    diffuseColor[3] = 1.0f;
}

static void renderQuad() {
    enableShader();
    setDiffuseColor(diffuseColor);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    disableShader();
}

static void updateCamera() {
    float fov_x = DEG_TO_RAD(100.0f);
    float fov_y = ((float) screenHeight) / ((float) screenWidth) * fov_x;
    float aspect = ((float) screenWidth) / ((float) screenHeight);
    float f = tanf(2.0f / fov_y);
    float far = 100.0f;
    float near = 0.05f;

    for (int i = 0; i < 16; ++i) {
        pMtx[0] = 0.0f;
    }

    pMtx[0] = f / aspect;
    pMtx[5] = f;
    pMtx[10] = (far + near) / (near - far);
    pMtx[11] = (2.0f * far * near) / (near - far);
    pMtx[14] = -1.0f;
}

int main() {
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

    glfwSwapInterval(1);
    glClearColor(0.25f, 0.25f, 0.75f, 1.0f);

    float prev_ts, cur_ts = 0.0f;
    while(!glfwWindowShouldClose(window)) {
        prev_ts = cur_ts;
        cur_ts = (float) glfwGetTime();
        float dt = cur_ts - prev_ts;

        glClear(GL_COLOR_BUFFER_BIT);

        update(dt);
        updateCamera();
        updateQuad(dt);

        renderQuad();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    deleteQuadModel();
    deleteShader();

    glfwTerminate();

    return 0;
}
