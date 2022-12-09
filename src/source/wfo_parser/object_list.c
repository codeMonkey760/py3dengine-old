#include <stdlib.h>
#include <string.h>

#include "wfo_parser/object_list.h"

#define INDEX_BUFFER_SIZE_IN_ELEMENTS 9

static const int name_buffer_size_in_elements = 64;

struct FaceListNode {
    struct FaceListNode *next;
    int dataIndices[9];
};

static void clearNameBuffer(char *nameBuffer) {
    memset(nameBuffer, 0, (name_buffer_size_in_elements+1) * sizeof(char) );
}

static void copyIndexBuffer(int dst[INDEX_BUFFER_SIZE_IN_ELEMENTS], const int src[INDEX_BUFFER_SIZE_IN_ELEMENTS]) {
    for (int i = 0; i < INDEX_BUFFER_SIZE_IN_ELEMENTS; ++i) {
        dst[i] = src[i];
    }
}

static void allocObjectListNode(struct ObjectListNode **objectListNodePtr) {
    if (objectListNodePtr == NULL || (*objectListNodePtr) != NULL) return;

    struct ObjectListNode *objectList = calloc(1, sizeof(struct ObjectListNode));
    if (objectList == NULL) return;

    objectList->next = NULL;
    objectList->faceList = NULL;
    objectList->name = calloc(name_buffer_size_in_elements + 1, sizeof(char));

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
    while (curNode != NULL) {
        curNode = curNode->next;
    }

    curNode->next = newNode;
}

void deleteObjectListNode(struct ObjectListNode **objectListNodePtr) {
    if (objectListNodePtr == NULL || (*objectListNodePtr) == NULL) return;

    struct ObjectListNode *objectList = (*objectListNodePtr);
    deleteObjectListNode(&(objectList->next));
    // TODO: delete face list?
    free(objectList->name);
    objectList->name = NULL;

    free(objectList);
    objectList = NULL;
    (*objectListNodePtr) = NULL;
}

void appendFaceToObjectList(struct ObjectListNode **objectListPtr, char *name, int indexBuffer[9]) {
    if (objectListPtr == NULL || name == NULL, indexBuffer == NULL) return;

    struct FaceListNode *newFaceListNode = calloc(1, sizeof(struct FaceListNode));
    if (newFaceListNode == NULL) return;

    newFaceListNode->next = NULL;
    copyIndexBuffer(newFaceListNode->dataIndices, indexBuffer);

    struct ObjectListNode *objectListNode = NULL;
    objectListNode = findObjectListNodeByName(objectListNode, name);
    if (objectListNode == NULL) {
        allocObjectListNode(&objectListNode);
        setObjectListNodeName(objectListNode, name);
        appendNodeToObjectList(objectListPtr, objectListNode);
    }

    // TODO: finish this
}