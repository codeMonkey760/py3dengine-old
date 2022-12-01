#include <glad/gl.h>

#include "quadmodel.h"

GLuint vao = -1;
GLuint vbo = -1;
GLuint posShaderIndex = 0;

void initQuadModel() {
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

void deleteQuadModel() {
    glDeleteBuffers(1, &vbo);
    vbo = -1;
    glDeleteVertexArrays(1, &vao);
    vao = -1;
}

void bindQuadModel() {
    glBindVertexArray(vao);
}

void renderQuadModel() {
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void unbindQuadModel() {
    glBindVertexArray(0);
}