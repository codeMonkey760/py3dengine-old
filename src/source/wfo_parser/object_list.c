#include <stdlib.h>
#include <string.h>

#include "wfo_parser/object_list.h"

#define INDEX_BUFFER_SIZE_IN_ELEMENTS 9

static const int name_buffer_size_in_elements = 64;

struct FaceListNode {
    struct FaceListNode *next;
    int dataIndices[INDEX_BUFFER_SIZE_IN_ELEMENTS];
};

static void clearNameBuffer(char *nameBuffer) {
    memset(nameBuffer, 0, (name_buffer_size_in_elements+1) * sizeof(char) );
}

static void allocObjectListNode(struct ObjectListNode **objectListNodePtr) {
    if (objectListNodePtr == NULL || (*objectListNodePtr) != NULL) return;

    struct ObjectListNode *objectList = calloc(1, sizeof(struct ObjectListNode));
    if (objectList == NULL) return;

    objectList->next = NULL;
    objectList->faceList = NULL;
    objectList->name = calloc(name_buffer_size_in_elements + 1, sizeof(char));
    objectList->indexBuffer = NULL;
    objectList->indexBufferSize = 0;

    (*objectListNodePtr) = objectList;
    objectList = NULL;
}

static void setObjectListNodeName(struct ObjectListNode *objectListNode, char *newName) {
    if (objectListNode == NULL || newName == NULL) return;

    clearNameBuffer(objectListNode->name);
    strncpy(objectListNode->name, newName, name_buffer_size_in_elements);
}

static struct ObjectListNode * findObjectListNodeByName(struct ObjectListNode *objectListNode, const char *name) {
    if (objectListNode == NULL || name == NULL) return NULL;

    struct ObjectListNode *curNode = objectListNode;
    while (curNode != NULL) {
        if (strncmp(curNode->name, name, name_buffer_size_in_elements) == 0) {
            return curNode;
        }

        curNode = curNode->next;
    }

    return NULL;
}

static void appendNodeToObjectList(struct ObjectListNode **objectListPtr, struct ObjectListNode *newNode) {
    if (objectListPtr == NULL || newNode == NULL) return;

    if ((*objectListPtr) == NULL) {
        (*objectListPtr) = newNode;

        return;
    }

    struct ObjectListNode *curNode = (*objectListPtr);
    while (curNode->next != NULL) {
        curNode = curNode->next;
    }

    curNode->next = newNode;
}

static void allocFaceListNode(struct FaceListNode **faceListNodePtr, const int indexBuffer[INDEX_BUFFER_SIZE_IN_ELEMENTS]) {
    if (faceListNodePtr == NULL || (*faceListNodePtr) != NULL || indexBuffer == NULL) return;

    struct FaceListNode *newFaceListNode = calloc(1, sizeof(struct FaceListNode));
    if (newFaceListNode == NULL) return;

    newFaceListNode->next = NULL;
    for (int i = 0; i < INDEX_BUFFER_SIZE_IN_ELEMENTS; ++i) {
        newFaceListNode->dataIndices[i] = indexBuffer[i];
    }

    (*faceListNodePtr) = newFaceListNode;
}

static void appendFaceListNode(struct FaceListNode **faceListNodePtr, struct FaceListNode *newNode) {
    if (faceListNodePtr == NULL || newNode == NULL) return;

    if ( (*faceListNodePtr) == NULL ) {
        (*faceListNodePtr) = newNode;

        return;
    }

    struct FaceListNode *curNode = (*faceListNodePtr);
    while (curNode->next != NULL) {
        curNode = curNode->next;
    }

    curNode->next = newNode;
}

static void deleteFaceList(struct FaceListNode **faceListNodePtr) {
    if (faceListNodePtr == NULL || (*faceListNodePtr) == NULL) return;

    struct FaceListNode *faceList = (*faceListNodePtr);
    deleteFaceList(&faceList->next);

    free(faceList);
    faceList = NULL;
    (*faceListNodePtr) = NULL;
}

static unsigned long getFaceCount(struct ObjectListNode *objectList) {
    if (objectList == NULL) return 0;

    struct FaceListNode *curNode = objectList->faceList;
    int faceCount = 0;

    while (curNode != NULL) {
        faceCount++;
        curNode = curNode->next;
    }

    return faceCount;
}

void deleteObjectListNode(struct ObjectListNode **objectListNodePtr) {
    if (objectListNodePtr == NULL || (*objectListNodePtr) == NULL) return;

    struct ObjectListNode *objectList = (*objectListNodePtr);
    deleteObjectListNode(&(objectList->next));
    deleteFaceList(&objectList->faceList);
    free(objectList->name);
    objectList->name = NULL;
    if (objectList->indexBuffer != NULL) {
        free(objectList->indexBuffer);
        objectList->indexBuffer = NULL;
    }
    objectList->indexBufferSize = 0;

    free(objectList);
    objectList = NULL;
    (*objectListNodePtr) = NULL;
}

void appendFaceToObjectList(struct ObjectListNode **objectListPtr, char *name, int indexBuffer[INDEX_BUFFER_SIZE_IN_ELEMENTS]) {
    if (objectListPtr == NULL || name == NULL || indexBuffer == NULL) return;

    struct FaceListNode *newFaceListNode = NULL;
    allocFaceListNode(&newFaceListNode, indexBuffer);
    if (newFaceListNode == NULL) return;

    struct ObjectListNode *objectListNode = NULL;
    objectListNode = findObjectListNodeByName((*objectListPtr), name);
    if (objectListNode == NULL) {
        allocObjectListNode(&objectListNode);
        if (objectListNode == NULL) {
            deleteFaceList(&newFaceListNode);

            return;
        }
        setObjectListNodeName(objectListNode, name);
        appendNodeToObjectList(objectListPtr, objectListNode);
    }

    appendFaceListNode(&objectListNode->faceList, newFaceListNode);
}

void flattenObjectList(struct ObjectListNode *objectList) {
    if (objectList == NULL || objectList->indexBuffer != NULL || objectList->faceList == NULL) return;

    size_t faceCount = getFaceCount(objectList);
    if (faceCount == 0) return;

    int *newIndexBuffer = calloc(faceCount * INDEX_BUFFER_SIZE_IN_ELEMENTS, sizeof(int));
    if (newIndexBuffer == NULL) return;

    struct FaceListNode *curNode = objectList->faceList;
    int count = 0;
    while (curNode != NULL && count < faceCount) {
        for (int i = 0; i < INDEX_BUFFER_SIZE_IN_ELEMENTS; ++i) {
            newIndexBuffer[(count * INDEX_BUFFER_SIZE_IN_ELEMENTS) + i] = curNode->dataIndices[i];
        }
        curNode = curNode->next;
        count++;
    }

    objectList->indexBuffer = newIndexBuffer;
    newIndexBuffer = NULL;
    objectList->indexBufferSize = faceCount * INDEX_BUFFER_SIZE_IN_ELEMENTS;
    deleteFaceList(&objectList->faceList);

    flattenObjectList(objectList->next);
}

int *getIndexBuffer(struct ObjectListNode *objectListNode) {
    if (objectListNode == NULL) return NULL;

    return objectListNode->indexBuffer;
}

size_t getIndexBufferSize(struct ObjectListNode *objectListNode) {
    if (objectListNode == NULL) return 0;

    return objectListNode->indexBufferSize;
}