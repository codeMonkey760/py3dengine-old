#include <stdlib.h>
#include <string.h>

#include "wfo_parser/vertex_data_list.h"

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