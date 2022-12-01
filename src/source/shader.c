#include <stdio.h>
#include <stdlib.h>

#include "shader.h"
#include "glad/gl.h"

static GLuint vertexShader = -1;
static GLuint fragShader = -1;
static GLuint program = -1;
static GLint diffuseColorIndex = -1;

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

void enableShader() {
    glUseProgram(program);
}

void disableShader() {
    glUseProgram(0);
}

void initShader() {
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

void deleteShader() {
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

void setDiffuseColor(float newDiffuseColor[4]) {
    glUniform4fv(diffuseColorIndex, 1, newDiffuseColor);
}
