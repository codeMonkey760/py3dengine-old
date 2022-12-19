#include <stdio.h>
#include <stdlib.h>

#include "logger.h"
#include "shader.h"
#include "glad/gl.h"

static GLuint vertexShader = -1;
static GLuint fragShader = -1;
static GLuint program = -1;

static GLint diffuseColorIndex = -1;
static GLint cameraPositionIndex = -1;
static GLint wMtxIndex = -1;
static GLint witMtxIndex = -1;
static GLint wvpMtxIndex = -1;

static const char *vertex_shader_source =
        "#version 460 core\n\n"

        "layout(location = 0) in vec3 posL;\n"
        "layout(location = 1) in vec3 normL;\n\n"

        "uniform mat4 gWMtx;\n"
        "uniform mat4 gWITMtx;\n"
        "uniform mat4 gWVPMtx;\n\n"

        "out vec3 posW;\n"
        "out vec3 normW;\n\n"

        "void main() {\n"
        "    posW = (vec4(posL, 1.0f) * gWMtx).xyz;\n"
        "    normW = (vec4(normL, 0.0f) * gWITMtx).xyz;\n\n"

        "    gl_Position = (vec4(posL, 1.0) * gWVPMtx);\n"
        "}\n";

static const char *fragment_shader_source =
        "#version 460 core\n\n"

        "in vec3 posW;\n"
        "in vec3 normW;\n\n"

        "uniform vec3 gDiffuseColor;\n"
        "uniform vec3 gCamPos;\n\n"

        "layout(location = 0) out vec4 outputColor;\n\n"

        "void main() {\n"
        "    vec3 normWFixed = normalize(normW);\n"
        "    vec3 toCamera = normalize(gCamPos - posW);\n\n"

        "    float lightValue = max(dot(toCamera, normWFixed), 0.0f);\n"
        "    lightValue = (lightValue * 0.7f) + 0.3f;\n\n"

        "    outputColor = vec4(gDiffuseColor * lightValue, 1.0f);\n"
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

static GLint getUniformIndex(char *name) {
    if (program == -1) return -1;

    GLint index = glGetUniformLocation(program, name);
    if (index == -1) {
        error_log("[Shader]: Unable to retrieve shader uniform location for %s", name);
    }

    return index;
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

    diffuseColorIndex = getUniformIndex("gDiffuseColor");
    cameraPositionIndex = getUniformIndex("gCamPos");
    wMtxIndex = getUniformIndex("gWMtx");
    witMtxIndex = getUniformIndex("gWITMtx");
    wvpMtxIndex = getUniformIndex("gWVPMtx");
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

void setDiffuseColor(float newDiffuseColor[3]) {
    glUniform3fv(diffuseColorIndex, 1, newDiffuseColor);
}

void setCameraPosition(float newCameraPos[3]) {
    glUniform3fv(cameraPositionIndex, 1, newCameraPos);
}

void setWMtx(float newWMtx[16]) {
    glUniformMatrix4fv(wMtxIndex, 1, GL_TRUE, newWMtx);
}

void setWITMtx(float newWITMtx[16]) {
    glUniformMatrix4fv(witMtxIndex, 1, GL_TRUE, newWITMtx);
}

void setWVPMtx(float newWVPMtx[16]) {
    glUniformMatrix4fv(wvpMtxIndex, 1, GL_TRUE, newWVPMtx);
}
