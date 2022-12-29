#include <stdio.h>
#include <stdlib.h>

#include "logger.h"
#include "custom_string.h"
#include "shader.h"
#include "glad/gl.h"

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

static GLint getUniformIndex(GLuint program, char *name) {
    if (program == -1 || name == NULL) return -1;

    GLint index = glGetUniformLocation(program, name);
    if (index == -1) {
        error_log("[Shader]: Unable to retrieve shader uniform location for %s", name);
    }

    return index;
}

static void deleteGLShader(struct Shader *shader, GLuint *glShaderPtr) {
    if (shader == NULL || glShaderPtr == NULL || (*glShaderPtr) == 0 || shader->_program == 0) return;

    GLuint glShader = (*glShaderPtr);

    glDetachShader(shader->_program, glShader);
    glDeleteShader(glShader);
    shader = 0;
    (*glShaderPtr) = 0;
}

void allocShader(struct Shader **shaderPtr) {
    if (shaderPtr == NULL || (*shaderPtr) != NULL) return;

    struct Shader *newShader = calloc(1, sizeof(struct Shader));
    if (newShader == NULL) return;

    newShader->_vertexShader = 0;
    newShader->_fragShader = 0;
    newShader->_program = 0;

    newShader->_diffuseColorLoc = -1;
    newShader->_cameraPositionLoc = -1;
    newShader->_wMtxLoc = -1;
    newShader->_witMtxLoc = -1;
    newShader->_wvpMtxLoc = -1;

    newShader->_name = NULL;

    (*shaderPtr) = newShader;
    newShader = NULL;
}

void deleteShader(struct Shader **shaderPtr) {
    if (shaderPtr == NULL || (*shaderPtr) == NULL) return;

    struct Shader *shader = (*shaderPtr);

    deleteGLShader(shader, &shader->_vertexShader);
    deleteGLShader(shader, &shader->_fragShader);

    glDeleteProgram(shader->_program);
    shader->_program = 0;

    shader->_diffuseColorLoc = -1;
    shader->_cameraPositionLoc = -1;
    shader->_wMtxLoc = -1;
    shader->_witMtxLoc = -1;
    shader->_wvpMtxLoc = -1;
    deleteString(&shader->_name);

    free(shader);
    shader = NULL;
    (*shaderPtr) = NULL;
}

void initShader(struct Shader *shader, const char *vertexShaderSource, const char *fragShaderSource) {
    if (shader == NULL || vertexShaderSource == NULL || fragShaderSource == NULL) return;

    GLint vs = glCreateShader(GL_VERTEX_SHADER);
    if (vs == 0) return;
    glShaderSource(vs, 1, &vertexShaderSource, 0);
    compileShader(vs);
    shader->_vertexShader = vs;
    vs = 0;

    GLint fs = glCreateShader(GL_FRAGMENT_SHADER);
    if (fs == 0) return;
    glShaderSource(fs, 1, &fragShaderSource, 0);
    compileShader(fs);
    shader->_fragShader = fs;
    fs = 0;

    GLint pgm = glCreateProgram();
    if (pgm == 0) return;
    glAttachShader(pgm, shader->_vertexShader);
    glAttachShader(pgm, shader->_fragShader);
    linkProgram(pgm);
    shader->_program = pgm;
    pgm = 0;

    shader->_diffuseColorLoc = getUniformIndex(shader->_program, "gDiffuseColor");
    shader->_cameraPositionLoc = getUniformIndex(shader->_program, "gCamPos");
    shader->_wMtxLoc = getUniformIndex(shader->_program, "gWMtx");
    shader->_witMtxLoc = getUniformIndex(shader->_program, "gWITMtx");
    shader->_wvpMtxLoc = getUniformIndex(shader->_program, "gWVPMtx");
}

void enableShader(struct Shader *shader) {
    if (shader == NULL) return;

    glUseProgram(shader->_program);
}

void disableShader(struct Shader *shader) {
    if (shader == NULL) return;

    glUseProgram(0);
}

void setDiffuseColor(struct Shader *shader, const float newDiffuseColor[3]) {
    if (shader == NULL || shader->_diffuseColorLoc == -1) return;

    glUniform3fv(shader->_diffuseColorLoc, 1, newDiffuseColor);
}

void setCameraPosition(struct Shader *shader, float newCameraPos[3]) {
    if (shader == NULL || shader->_cameraPositionLoc == -1) return;

    glUniform3fv(shader->_cameraPositionLoc, 1, newCameraPos);
}

void setWMtx(struct Shader *shader, float newWMtx[16]) {
    if (shader == NULL || shader->_wMtxLoc == -1) return;

    glUniformMatrix4fv(shader->_wMtxLoc, 1, GL_TRUE, newWMtx);
}

void setWITMtx(struct Shader *shader, float newWITMtx[16]) {
    if (shader == NULL || shader->_witMtxLoc == -1) return;

    glUniformMatrix4fv(shader->_witMtxLoc, 1, GL_TRUE, newWITMtx);
}

void setWVPMtx(struct Shader *shader, float newWVPMtx[16]) {
    if (shader == NULL || shader->_wvpMtxLoc == -1) return;

    glUniformMatrix4fv(shader->_wvpMtxLoc, 1, GL_TRUE, newWVPMtx);
}

struct String *getShaderName(struct Shader *shader) {
    if (shader == NULL || shader->_name == NULL) return NULL;

    return shader->_name;
}

void setShaderName(struct Shader *shader, const char *newName) {
    if (shader == NULL) return;

    if (newName == NULL) {
        deleteString(&shader->_name);

        return;
    }

    if (shader->_name == NULL) {
        allocString(&shader->_name, newName);
    } else {
        setChars(shader->_name, newName);
    }
}