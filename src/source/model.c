#include <string.h>

#include "glad/gl.h"
#include "custom_string.h"
#include "model.h"

// TODO: these should be in a vertex format file / struct
const GLuint positionShaderIndex = 0;
const GLuint normalShaderIndex = 1;
const GLuint texCoordShaderIndex = 2;

static void deleteVAO(struct Model *model) {
    if (model == NULL) return;

    if (model->_vao != -1) {
        glDeleteVertexArrays(1, &model->_vao);
        model->_vao = -1;
    }
}

static void deleteVBO(struct Model *model) {
    if (model == NULL) return;

    if (model->_vbo != -1) {
        glDeleteBuffers(1, &model->_vbo);
        model->_vbo = -1;
    }
    model->_sizeInVertices = 0;
}

void allocModel(struct Model **modelPtr) {
    if (modelPtr == NULL || (*modelPtr) != NULL) return;

    struct Model *newModel = calloc(1, sizeof(struct Model));
    newModel->_vao = -1;
    newModel->_vbo = -1;
    newModel->_sizeInVertices = 0;

    newModel->_name = NULL;

    (*modelPtr) = newModel;
    newModel = NULL;
}

void deleteModel(struct Model **modelPtr) {
    if (modelPtr == NULL || (*modelPtr) == NULL) return;

    struct Model *model = (*modelPtr);

    deleteVBO(model);
    deleteVAO(model);
    deleteString(&model->_name);

    free(model);
    model = NULL;
    (*modelPtr) = NULL;
}

// TODO: custom vertex formats?
void setPNTBuffer(struct Model *model, const float *buffer, size_t bufferSizeInElements) {
    if (model == NULL || buffer == NULL || bufferSizeInElements == 0) return;

    GLuint newVao = -1;
    glGenVertexArrays(1, &newVao);
    if (newVao == -1) return;

    glBindVertexArray(newVao);

    GLuint newVbo = -1;
    glGenBuffers(1, &newVbo);
    if (newVbo == -1) {
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &newVao);
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, newVbo);
    glBufferData(GL_ARRAY_BUFFER, ((long) bufferSizeInElements) * 3 * 96, buffer, GL_STATIC_DRAW);

    glEnableVertexAttribArray(positionShaderIndex);
    glEnableVertexAttribArray(normalShaderIndex);
    glEnableVertexAttribArray(texCoordShaderIndex);

    glVertexAttribPointer(positionShaderIndex, 3, GL_FLOAT, GL_FALSE, 32, (const void *) 0);
    glVertexAttribPointer(normalShaderIndex, 3, GL_FLOAT, GL_FALSE, 32, (const void *) 12);
    glVertexAttribPointer(texCoordShaderIndex, 2, GL_FLOAT, GL_FALSE, 32, (const void *) 24);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    deleteVAO(model);
    deleteVBO(model);

    model->_vao = newVao;
    model->_vbo = newVbo;
    model->_sizeInVertices = bufferSizeInElements;
}

void bindModel(struct Model *model) {
    if (model == NULL || model->_vao == -1) return;

    glBindVertexArray(model->_vao);
}

void unbindModel(struct Model *model) {
    if (model == NULL) return;

    glBindVertexArray(0);
}

void renderModel(struct Model *model) {
    if (model == NULL || model->_vao == -1 || model->_sizeInVertices == 0) return;

    glDrawArrays(GL_TRIANGLES, 0, (int) model->_sizeInVertices);
}

struct String *getModelName(struct Model *model) {
    if (model->_name == NULL) return NULL;

    return model->_name;
}

void setModelName(struct Model *model, const char *newName) {
    if (model == NULL) return;

    if (newName == NULL) {
        deleteString(&model->_name);
        return;
    }

    if (model->_name == NULL) {
        allocString(&model->_name, newName);
    } else {
        setChars(model->_name, newName);
    }
}
