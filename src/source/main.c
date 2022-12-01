#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "../headers/util.h"

GLuint vertexShader = -1;
GLuint fragShader = -1;
GLuint program = -1;
GLint diffuseColorIndex = -1;

GLuint vao = -1;
GLuint vbo = -1;
GLuint posShaderIndex = 0;

int screenWidth = 800;
int screenHeight = 600;
float diffuseColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

float pMtx[16] = {0.0f};

static const char *vertex_shader_source =
        "#version 460 core\n"
        "layout(location = 0) in vec3 posL;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(posL, 1.0);\n"
        "}\n";

static const char *fragment_shader_source =
        "#version 460 core\n"
        "uniform vec4 gDiffuseColor;\n"
        "layout(location = 0) out vec4 outputColor;\n"
        "void main()\n"
        "{\n"
        "   outputColor = gDiffuseColor;\n"
        "}\n";

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

static void compileShader(GLuint shader) {
    GLint result = GL_FALSE;
    GLint log_length = 0;

    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_TRUE) {
        return;
    }

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length <= 0) {
        fprintf(stderr, "Shader failed to compile but didn't have a log");

        return;
    }
    char *shader_log = calloc(log_length, sizeof(char));

    glGetShaderInfoLog(shader, log_length, 0, shader_log);

    GLint shader_type = -1;
    glGetShaderiv(shader, GL_SHADER_TYPE, &shader_type);
    const char *shader_type_name = "Unknown";
    if (shader_type == GL_VERTEX_SHADER) {
        shader_type_name = "Vertex";
    } else if (shader_type == GL_FRAGMENT_SHADER) {
        shader_type_name = "Fragment";
    }

    fprintf(stderr, "%s shader failed to compile:\n", shader_type_name);
    fprintf(stderr, "%s\n", shader_log);

    free(shader_log);
    shader_log = NULL;
}

static void linkProgram(GLuint pgm) {
    GLint result = GL_FALSE;
    GLint log_length = 0;

    glLinkProgram(pgm);

    glGetProgramiv(pgm, GL_LINK_STATUS, &result);
    if (result == GL_TRUE) {
        return;
    }

    glGetProgramiv(pgm, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length <= 0) {
        fprintf(stderr, "Program failed to link but didn't have a log");

        return;
    }

    char *link_log = calloc(log_length, sizeof(char));
    glGetProgramInfoLog(pgm, log_length, NULL, link_log);

    fprintf(stderr, "Program failed to link:\n");
    fprintf(stderr, "%s\n", link_log);

    free(link_log);
    link_log = NULL;
}

static void initShader() {
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertex_shader_source, 0);
    compileShader(vertexShader);

    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragment_shader_source, 0);
    compileShader(fragShader);

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragShader);
    linkProgram(program);

    diffuseColorIndex = glGetUniformLocation(program, "gDiffuseColor");
}

static void deleteShader() {
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragShader);
    glDeleteShader(vertexShader);
    vertexShader = -1;
    glDeleteShader(fragShader);
    fragShader = -1;
    diffuseColorIndex = -1;
    glDeleteProgram(program);
    program = -1;
}

static void renderQuad() {
    glUseProgram(program);
    glUniform4fv(diffuseColorIndex, 1, diffuseColor);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glUseProgram(0);
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
