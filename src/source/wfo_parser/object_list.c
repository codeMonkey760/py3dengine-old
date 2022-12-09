#include <stdlib.h>
#include <string.h>

#include "wfo_parser/object_list.h"

static const int name_buffer_size_in_elements = 64;

struct FaceListNode {
    struct FaceListNode *next;
    unsigned int dataIndices[9];
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

    (*objectListNodePtr) = objectList;
    objectList = NULL;
}

static void setObjectListNodeName(struct ObjectListNode *objectListNode, char *newName) {
    if (objectListNode == NULL || newName == NULL) return;

    clearNameBuffer(objectListNode->name);
    strncpy(objectListNode->name, newName, name_buffer_size_in_elements);
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


}