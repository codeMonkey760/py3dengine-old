#include <glad/glad.h>
#include "custom_string.h"
#include "resources/model.h"

#define RESOURCE_TYPE_MODEL 2

struct VertexPNT {
    float position[3];
    float normal[3];
    float texCoord[2];
};

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

static void delete(struct BaseResource **resourcePtr) {
    if (resourcePtr == NULL) return;

    if (!isResourceTypeModel((*resourcePtr))) return;

    deleteModel((struct Model **) resourcePtr);
}

bool isResourceTypeModel(struct BaseResource *resource) {
    if (resource == NULL) return false;

    return resource->_type == RESOURCE_TYPE_MODEL && stringEqualsCStr(resource->_typeName, RESOURCE_TYPE_NAME_MODEL);
}

void allocModel(struct Model **modelPtr) {
    if (modelPtr == NULL || (*modelPtr) != NULL) return;

    struct Model *newModel = calloc(1, sizeof(struct Model));
    if (newModel == NULL) return;

    struct BaseResource *base = (struct BaseResource *) newModel;
    initializeBaseResource(base);

    base->_type = RESOURCE_TYPE_MODEL;
    allocString(&base->_typeName, RESOURCE_TYPE_NAME_MODEL);
    base->delete = delete;
    base = NULL;

    newModel->_vao = -1;
    newModel->_vbo = -1;
    newModel->_sizeInVertices = 0;

    (*modelPtr) = newModel;
    newModel = NULL;
}

void deleteModel(struct Model **modelPtr) {
    if (modelPtr == NULL || (*modelPtr) == NULL) return;

    struct Model *model = (*modelPtr);

    deleteVBO(model);
    deleteVAO(model);

    finalizeBaseResource((struct BaseResource *) model);

    free(model);
    model = NULL;
    (*modelPtr) = NULL;
}

// TODO: custom vertex formats?
void setModelPNTBuffer(struct Model *model, struct VertexPNT *buffer, size_t bufferSizeInVertices) {
    if (model == NULL || buffer == NULL || bufferSizeInVertices == 0) return;

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
    size_t bufferSizeInBytes = (sizeof(struct VertexPNT)) * bufferSizeInVertices;
    glBufferData(GL_ARRAY_BUFFER, bufferSizeInBytes, buffer, GL_STATIC_DRAW);

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
    model->_sizeInVertices = bufferSizeInVertices;
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
