#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "custom_string.h"
#include "wfo_parser/vertex_data_list.h"
#include "wfo_parser/wfo_parser.h"

static const int line_buffer_size_in_elements = 256;

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
    if (curPos == NULL || (*curPos) == 0 || dst == NULL || limit < 0) return curPos;

    for (int i = 0; i < limit; ++i) {
        char curChar = (*curPos);
        if (isspace(curChar) || curChar == 0) {
            return curPos;
        }
        (*dst) = curChar;
        curPos++;
    }

    return curPos;
}

static char* readFloatFromLine(char *curPos, float *dst) {
    if (curPos == NULL || (*curPos) == 0 || dst == NULL) return curPos;

    char *newEnd = NULL;
    (*dst) = strtof(curPos, &newEnd);

    return newEnd;
}

static char* advancePastSpaces(char *curPos) {
    if (curPos == NULL || (*curPos) == 0) return curPos;

    while ( (*curPos) == ' ' ) {
        curPos++;
    }

    return curPos;
}

static void readFloatsFromLine(char *curPos, float *dataBuffer, int numFloatsToRead) {
    if (curPos == NULL || (*curPos) == 0 || dataBuffer == NULL || numFloatsToRead < 1 || numFloatsToRead > 3) return;

    for (int i = 0; i < numFloatsToRead; ++i) {
        curPos = readFloatFromLine(curPos, &(dataBuffer[i]));
        curPos = advancePastSpaces(curPos);
    }
}

void parseWaveFrontFile(FILE *wfo, struct Model **modelPtr) {
    if (wfo == NULL) return;

    struct VertexListNode *posList = NULL, *normalList = NULL, *texCoordList = NULL;
    struct ObjectListNode *objectList = NULL;
    struct String *curObjectName = NULL;

    char *lineBuffer = NULL;
    allocLineBuffer(&lineBuffer);
    char *typeBuffer = calloc(3, sizeof(char));
    char *curPos = NULL;
    float *dataBuffer = calloc(3, sizeof(float));
    char *nameBuffer = calloc(65, sizeof(char));
    allocString(&curObjectName, "None");

    while(fgets(lineBuffer, line_buffer_size_in_elements, wfo) != NULL) {
        curPos = lineBuffer;
        curPos = readStringFromLine(curPos, typeBuffer, 2);
        curPos = advancePastSpaces(curPos);

        if (strncmp(typeBuffer, "v", 1) == 0) {
            readFloatsFromLine(curPos, dataBuffer, 3);
            appendVertexData(&posList, typeBuffer, dataBuffer, 3);
        } else if (strncmp(typeBuffer, "vn", 2) == 0) {
            readFloatsFromLine(curPos, dataBuffer, 3);
            appendVertexData(&normalList, typeBuffer, dataBuffer, 3);
        } else if (strncmp(typeBuffer, "vt", 2) == 0) {
            readFloatsFromLine(curPos, dataBuffer, 2);
            appendVertexData(&texCoordList, typeBuffer, dataBuffer, 2);
        } else if (strncmp(typeBuffer, "o", 1) == 0) {
            memset(nameBuffer, 0, 65 * sizeof(char));
            readStringFromLine(curPos, nameBuffer, 64);
            setChars(curObjectName, nameBuffer);
        }

        resetLineBuffer(lineBuffer);
    }

    deleteString(&curObjectName);
    deleteVertexDataList(&texCoordList);
    deleteVertexDataList(&normalList);
    deleteVertexDataList(&posList);

    free(dataBuffer);
    dataBuffer = NULL;
    free(typeBuffer);
    typeBuffer = NULL;
    deleteLineBuffer(&lineBuffer);
}