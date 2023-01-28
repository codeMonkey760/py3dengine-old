#include <glad/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "custom_string.h"
#include "resources/shader.h"
#include "resources/texture.h"

#define RESOURCE_TYPE_SHADER 3

struct UniformListNode {
    struct String *name;
    GLint location;
    struct UniformListNode *next;
};

static void allocUniformListNode(struct UniformListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) != NULL) return;

    struct UniformListNode *newNode = calloc(1, sizeof(struct UniformListNode));
    if (newNode == NULL) return;
    newNode->next = NULL;
    newNode->name = NULL;
    newNode->location = -1;

    (*listNodePtr) = newNode;
    newNode = NULL;
}

static void initUniformListNode(
    struct UniformListNode *listNode,
    GLint location,
    const char *name
) {
    if (listNode == NULL || name == NULL || location == -1) return;

    listNode->location = location;
    if(listNode->name != NULL) {
        setChars(listNode->name, name);
    } else {
        allocString(&listNode->name, name);
    }
}

static void deleteUniformListNode(struct UniformListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) == NULL) return;

    struct UniformListNode *node = (*listNodePtr);
    deleteUniformListNode(&node->next);
    deleteString(&node->name);

    free(node);
    node = NULL;
    (*listNodePtr) = NULL;
}

static void storeShaderUniformInfo(struct Shader *shader, GLint location, const char *name) {
    if (shader == NULL || location == -1 || name == NULL) return;

    struct UniformListNode *prevNode = NULL, *curNode = shader->uniformList;
    while(curNode != NULL) {
        prevNode = curNode;
        curNode = curNode->next;
    }

    struct UniformListNode *newNode = NULL;
    allocUniformListNode(&newNode);
    if (newNode == NULL) return;
    initUniformListNode(newNode, location, name);

    if (prevNode == NULL) {
        shader->uniformList = newNode;
    } else {
        prevNode->next = newNode;
    }
    newNode = NULL;
}

static GLint getUniformLocation(struct Shader *shader, const char *name) {
    if (shader == NULL || name == NULL) return -1;

    struct UniformListNode *curNode = shader->uniformList;
    while (curNode != NULL) {
        if (stringEqualsCStr(curNode->name, name)) break;
        curNode = curNode->next;
    }

    if (curNode == NULL) return -1;

    return curNode->location;
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

static void queryShaderUniformLocations(struct Shader *shader) {
    if (shader == NULL || shader->_program == 0) return;

    GLint numUniforms = 0;
    glGetProgramiv(shader->_program, GL_ACTIVE_UNIFORMS, &numUniforms);
    if (numUniforms == 0) return;

    for (GLuint i = 0; i < numUniforms; ++i) {
        char nameBuffer[64];
        memset(nameBuffer, 0, sizeof(char) * 64);
        GLsizei uniformSize = 0;
        GLenum uniformType = 0;
        GLsizei nameLength = 0;
        glGetActiveUniform(shader->_program, i, 63, &nameLength, &uniformSize, &uniformType, nameBuffer);

        storeShaderUniformInfo(shader, i, nameBuffer);
    }
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

static void delete(struct BaseResource **resourcePtr) {
    if (resourcePtr == NULL) return;

    if (!isResourceTypeShader( (*resourcePtr) ))  return;

    deleteShader((struct Shader **) resourcePtr);
}

static char *getFileContents(const char *fileName) {
    char *buffer = NULL;
    size_t length = 0;

    FILE *f = fopen(fileName, "r");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = calloc(length+1, sizeof(char));
    if (buffer != NULL) {
        fread(buffer, sizeof(char), length, f);
        buffer[length] = 0;
    }
    fclose(f);
    f = NULL;

    return buffer;
}

bool isResourceTypeShader(struct BaseResource *resource) {
    if (resource == NULL) return false;

    return resource->_type == RESOURCE_TYPE_SHADER && stringEqualsCStr(resource->_typeName, RESOURCE_TYPE_NAME_SHADER);
}

void allocShader(struct Shader **shaderPtr) {
    if (shaderPtr == NULL || (*shaderPtr) != NULL) return;

    struct Shader *newShader = calloc(1, sizeof(struct Shader));
    if (newShader == NULL) return;

    struct BaseResource *base = (struct BaseResource *) newShader;
    initializeBaseResource(base);
    base->_type = RESOURCE_TYPE_SHADER;
    allocString(&base->_typeName, RESOURCE_TYPE_NAME_SHADER);
    base->delete = delete;
    base = NULL;

    newShader->_vertexShader = 0;
    newShader->_fragShader = 0;
    newShader->_program = 0;

    newShader->uniformList = NULL;

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

    deleteUniformListNode(&shader->uniformList);

    finalizeBaseResource((struct BaseResource *) shader);

    free(shader);
    shader = NULL;
    (*shaderPtr) = NULL;
}

bool setShaderFloatArrayUniform(struct Shader *shader, const char *name, const float *src, size_t numElements) {
    if (shader == NULL || name == NULL || src == NULL || numElements == 0) return false;

    if (numElements > 4) return false;

    GLint loc = getUniformLocation(shader, name);
    if (loc == -1) return false;

    switch (numElements) {
    case 1:
        glUniform1fv(loc, 1, src);
        break;
    case 2:
        glUniform2fv(loc, 1, src);
        break;
    case 3:
        glUniform3fv(loc, 1, src);
        break;
    case 4:
        glUniform4fv(loc, 1, src);
        break;
    default:
        return false;
    }

    return true;
}

bool setShaderMatrixUniform(struct Shader *shader, const char *name, const float matrix[16]) {
    if (shader == NULL || name == NULL || matrix == NULL) return false;

    GLint loc = getUniformLocation(shader, name);
    if (loc == -1) return false;

    glUniformMatrix4fv(loc, 1, GL_TRUE, matrix);

    return true;
}

bool setShaderTextureUniform(struct Shader *shader, const char *name, struct Texture *texture) {
    if (shader == NULL || name == NULL || texture == NULL) return false;

    if (!glIsTexture(texture->_id)) return false;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->_id);

    GLint loc = getUniformLocation(shader, name);
    if (loc == -1) return false;

    glUniform1i(loc, 0);
    if (glGetError() != GL_NO_ERROR) {
        warning_log("[Shader]: OpenGL raised error when setting \"%s\" shader param", name);
    }

    return true;
}

void initShaderFromFiles(struct Shader *shader, const char *vs_filename, const char *fs_filename) {
    if (shader == NULL || vs_filename == NULL || fs_filename == NULL) return;

    char *vs_source = getFileContents(vs_filename);
    if (vs_source == NULL) {
        error_log("[Shader]: Could not open \"%s\" for reading", vs_filename);
        return;
    }

    char *fs_source = getFileContents(fs_filename);
    if (fs_source == NULL) {
        error_log("[Shader]: Could not open \"%s\" for reading", fs_filename);
        free(vs_source);
        vs_source = NULL;
        return;
    }

    initShader(shader, vs_source, fs_source);

    free(vs_source);
    vs_source = NULL;
    free(fs_source);
    fs_source = NULL;
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

    queryShaderUniformLocations(shader);
}

void enableShader(struct Shader *shader) {
    if (shader == NULL) return;

    glUseProgram(shader->_program);
}

void disableShader(struct Shader *shader) {
    if (shader == NULL) return;

    glUseProgram(0);
}
