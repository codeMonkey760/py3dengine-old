#include <stdlib.h>
#include <string.h>

#include "custom_string.h"
#include "wfo_parser.h"

struct VertexListNode {
    struct VertexList *next;
    float data[3];
    char type[3];
    int data_len; //number of floats used from data array
};

struct FaceListNode {
    struct FaceListNode *next;
    unsigned int dataIndices[9];
};

struct ObjectListNode {
    struct ObjectListNode *next;
    struct FaceListNode *faceList;
    struct String *name;
};

static const int line_buffer_size_in_elements = 256;

static void allocObjectListNode(struct ObjectListNode **objectListNodePtr) {
    if (objectListNodePtr == NULL || (*objectListNodePtr) != NULL) return;

    struct ObjectListNode *objectList = calloc(1, sizeof(struct ObjectListNode));
    if (objectList == NULL) return;

    objectList->next = NULL;
    objectList->faceList = NULL;
    objectList->name = NULL;

    (*objectListNodePtr) = objectList;
    objectList = NULL;
}

static void deleteObjectListNode(struct ObjectListNode **objectListNodePtr) {
    if (objectListNodePtr == NULL || (*objectListNodePtr) == NULL) return;

    struct ObjectListNode *objectList = (*objectListNodePtr);
    deleteObjectListNode(&(objectList->next));
    // TODO: delete face list?
    deleteString(&(objectList->name));

    free(objectList);
    objectList = NULL;
    (*objectListNodePtr) = NULL;
}

static void setObjectListNodeName(struct ObjectListNode *objectList, char *newName) {
    if (objectList == NULL || newName == NULL) return;

    if (objectList->name == NULL) {
        allocString(&(objectList->name), newName);
    } else {
        setChars(objectList->name, newName);
    }
}

static void allocLineBuffer(char **lineBufferPtr) {
    if (lineBufferPtr == NULL || (*lineBufferPtr) != NULL) return;

    char *newLineBuffer = calloc(line_buffer_size_in_elements, sizeof(char));

    (*lineBufferPtr) = newLineBuffer;
}

static void deleteLineBuffer(char **lineBufferPtr) {
    if (lineBufferPtr == NULL || (*lineBufferPtr) == NULL) return;

    free( (*lineBufferPtr) );
    (*lineBufferPtr) = NULL;
}

static void resetLineBuffer(char *lineBuffer) {
    memset(lineBuffer, line_buffer_size_in_elements, sizeof(char));
}

static char* readStringFromLine(char *curPos, char *dst, int limit) {
    if (curPos == NULL || dst == NULL || limit < 0) return curPos;

    for (int i = 0; i < limit; ++i) {
        (*dst) = (*curPos++);
        if ((*curPos) == ' ' || (*curPos) == '\n' || (*curPos) == '\r') {
            return curPos;
        }
    }

    return curPos;
}

static char* advancePastSpaces(char *curPos) {
    if (curPos == NULL) return NULL;

    while ( (*curPos) == ' ' ) {
        curPos++;
    }

    return curPos;
}

void parseWaveFrontFile(FILE *wfo, struct Model **modelPtr) {
    if (modelPtr == NULL || (*modelPtr) != NULL || wfo == NULL) return;

    struct VertexListNode *posList = NULL, *lastPosNode = NULL;
    struct VertexListNode *normalList = NULL, *lastNormalNode = NULL;
    struct VertexListNode *texCoordList = NULL, *lastTexCoordNode = NULL;

    struct ObjectListNode *objectList = NULL;
    struct FaceListNode *curFaceList = NULL, *lastCurFaceNode = NULL;

    char *lineBuffer = NULL;
    allocLineBuffer(&lineBuffer);
    char *typeBuffer = calloc(3, sizeof(char));
    char *curPos = NULL;
    float *dataBuffer = calloc(3, sizeof(float));
    int data_used = 0;

    while(fgets(lineBuffer, line_buffer_size_in_elements, wfo) != NULL) {
        curPos = lineBuffer;
        curPos = readStringFromLine(curPos, typeBuffer, 2);
        curPos = advancePastSpaces(curPos);
        curPos = readFloatFromLine(curPos, )
    }

    free(dataBuffer);
    dataBuffer = NULL;
    free(typeBuffer);
    typeBuffer = NULL;
    deleteLineBuffer(&lineBuffer);
}