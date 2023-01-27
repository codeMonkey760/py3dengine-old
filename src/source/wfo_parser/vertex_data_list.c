#include <stdlib.h>

#include "wfo_parser/vertex_data_list.h"

static void allocVectorListNode(struct VectorListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) != NULL) return;

    struct VectorListNode *newNode = calloc(1, sizeof(struct VectorListNode));
    if (newNode == NULL) return;

    newNode->next = NULL;
    newNode->size = 0;
    newNode->elements[0] = 0.0f;
    newNode->elements[1] = 0.0f;
    newNode->elements[2] = 0.0f;

    (*listNodePtr) = newNode;
    newNode = NULL;
};

static struct VectorListNode *getVectorListEnd(struct VectorListNode *list) {
    struct VectorListNode *prevNode = NULL, *curNode = list;
    while (curNode != NULL) {
        prevNode = curNode;
        curNode = curNode->next;
    }

    if (prevNode == NULL) {
        return list;
    } else {
        return prevNode;
    }
}

void appendVector2(struct VectorListNode **vecListNodePtr, float x, float y) {
    if (vecListNodePtr == NULL) return;

    struct VectorListNode *prevNode = getVectorListEnd((*vecListNodePtr));
    struct VectorListNode *newNode = NULL;
    allocVectorListNode(&newNode);
    if (newNode == NULL) return;

    newNode->size = 2;
    newNode->elements[0] = x;
    newNode->elements[1] = y;

    if (prevNode == NULL && (*vecListNodePtr) == NULL) {
        (*vecListNodePtr) = newNode;
    } else {
        prevNode->next = newNode;
    }
    newNode = NULL;
}

void appendVector3(struct VectorListNode **vecListNodePtr, float x, float y, float z) {
    if (vecListNodePtr == NULL) return;

    struct VectorListNode *prevNode = getVectorListEnd((*vecListNodePtr));
    struct VectorListNode *newNode = NULL;
    allocVectorListNode(&newNode);
    if (newNode == NULL) return;

    newNode->size = 3;
    newNode->elements[0] = x;
    newNode->elements[1] = y;
    newNode->elements[2] = z;

    if (prevNode == NULL && (*vecListNodePtr) == NULL) {
        (*vecListNodePtr) = newNode;
    } else {
        prevNode->next = newNode;
    }
    newNode = NULL;
}

void deleteVectorList(struct VectorListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) == NULL) return;

    struct VectorListNode *node = (*listNodePtr);
    deleteVectorList(&node->next);

    free(node);
    node = NULL;
    (*listNodePtr) = NULL;
}

void flattenVectorList(struct VectorListNode *vecList, float **dstPtr, size_t *dstSizePtr) {
    if (vecList == NULL || dstPtr == NULL || (*dstPtr) != NULL || dstSizePtr == NULL) return;

    size_t floatCount = 0;
    struct VectorListNode *curNode = vecList;
    while (curNode != NULL) {
        floatCount += curNode->size;
        curNode = curNode->next;
    }

    float *vecBuffer = calloc(floatCount, sizeof(float));
    size_t curFloat = 0;
    curNode = vecList;
    while (curNode != NULL) {
        for (size_t i = 0; i < curNode->size; ++i) {
            vecBuffer[curFloat++] = curNode->elements[i];
        }
        curNode = curNode->next;
    }

    (*dstPtr) = vecBuffer;
    vecBuffer = NULL;
    (*dstSizePtr) = floatCount;
}
