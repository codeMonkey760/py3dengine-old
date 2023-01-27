#include <stdlib.h>
#include <string.h>

#include "wfo_parser/vertex_data_list.h"

static void allocVec2ListNode (struct Vec2ListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) != NULL) return;

    struct Vec2ListNode *newNode = calloc(1, sizeof(struct Vec2ListNode));
    if (newNode == NULL) return;

    newNode->next = NULL;
    newNode->elements[0] = 0.0f;
    newNode->elements[1] = 0.0f;

    (*listNodePtr) = newNode;
    newNode = NULL;
};

void appendVec2(struct Vec2ListNode **vec2ListNodePtr, float x, float y) {
    if (vec2ListNodePtr == NULL) return;

    struct Vec2ListNode *prevNode = NULL, *curNode = (*vec2ListNodePtr);
    while (curNode != NULL) {
        prevNode = curNode;
        curNode = curNode->next;
    }

    struct Vec2ListNode *newNode = NULL;
    allocVec2ListNode(&newNode);
    if (newNode == NULL) return;

    newNode->elements[0] = x;
    newNode->elements[1] = y;
}

void appendVertexData(struct VertexListNode **vertexDataListPtr, char *type, float *src, int size) {
    if (vertexDataListPtr == NULL || type == NULL || src == NULL || size > 3) return;

    struct VertexListNode *newNode = NULL;
    newNode = calloc(1, sizeof(struct VertexListNode));
    if (newNode == NULL) return;

    for (int i = 0; i < size; ++i) {
        newNode->data[i] = src[i];
    }
    newNode->data_len = size;
    strncpy(newNode->type, type, 2);
    newNode->type[2] = 0;
    newNode->next = NULL;

    struct VertexListNode *prevNode = NULL, *curNode = NULL;
    prevNode = curNode = (*vertexDataListPtr);
    while(curNode != NULL) {
        prevNode = curNode;
        curNode = curNode->next;
    }

    if (prevNode == NULL) {
        (*vertexDataListPtr) = newNode;
    } else {
        prevNode->next = newNode;
    }
}

void deleteVertexDataList(struct VertexListNode **vertexDataListPtr) {
    if (vertexDataListPtr == NULL || (*vertexDataListPtr) == NULL) return;

    struct VertexListNode *curNode = NULL, *temp = NULL;
    curNode = (*vertexDataListPtr);

    while (curNode != NULL) {
        temp = curNode->next;
        curNode->next = NULL;

        free(curNode);
        curNode = temp;
    }

    (*vertexDataListPtr) = NULL;
}

void printVertexDataList(FILE *fd, struct VertexListNode *vertexDataList) {
    if (fd == NULL) return;

    if (vertexDataList == NULL) {
        fprintf(fd, "List was empty\n");

        return;
    }

    struct VertexListNode *curNode = vertexDataList;
    while (curNode != NULL) {
        fprintf(fd, "%s ", curNode->type);

        for (int i = 0; i < curNode->data_len; ++i) {
            fprintf(fd, "%f ", curNode->data[i]);
        }

        fprintf(fd, "\n");

        curNode = curNode->next;
    }
}