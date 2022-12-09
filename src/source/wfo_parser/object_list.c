#include <stdlib.h>

#include "custom_string.h"
#include "wfo_parser/object_list.h"

struct FaceListNode {
    struct FaceListNode *next;
    unsigned int dataIndices[9];
};

void allocObjectListNode(struct ObjectListNode **objectListNodePtr) {
    if (objectListNodePtr == NULL || (*objectListNodePtr) != NULL) return;

    struct ObjectListNode *objectList = calloc(1, sizeof(struct ObjectListNode));
    if (objectList == NULL) return;

    objectList->next = NULL;
    objectList->faceList = NULL;
    objectList->name = NULL;

    (*objectListNodePtr) = objectList;
    objectList = NULL;
}

void deleteObjectListNode(struct ObjectListNode **objectListNodePtr) {
    if (objectListNodePtr == NULL || (*objectListNodePtr) == NULL) return;

    struct ObjectListNode *objectList = (*objectListNodePtr);
    deleteObjectListNode(&(objectList->next));
    // TODO: delete face list?
    deleteString(&(objectList->name));

    free(objectList);
    objectList = NULL;
    (*objectListNodePtr) = NULL;
}

void setObjectListNodeName(struct ObjectListNode *objectList, char *newName) {
    if (objectList == NULL || newName == NULL) return;

    if (objectList->name == NULL) {
        allocString(&(objectList->name), newName);
    } else {
        setChars(objectList->name, newName);
    }
}