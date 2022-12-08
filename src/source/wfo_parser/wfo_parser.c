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
        (*dst) = (*curPos++);
        if ((*curPos) == ' ' || (*curPos) == '\n' || (*curPos) == '\r') {
            return curPos;
        }
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

void parseWaveFrontFile(FILE *wfo, struct Model **modelPtr) {
    if (modelPtr == NULL || (*modelPtr) != NULL || wfo == NULL) return;

    struct VertexListNode *posList = NULL, *normalList = NULL, *texCoordList = NULL;
    struct ObjectListNode *objectList = NULL;
    struct String *curObjectName = NULL;

    char *lineBuffer = NULL;
    allocLineBuffer(&lineBuffer);
    char *typeBuffer = calloc(3, sizeof(char));
    char *curPos = NULL;
    float *dataBuffer = calloc(3, sizeof(float));

    while(fgets(lineBuffer, line_buffer_size_in_elements, wfo) != NULL) {
        curPos = lineBuffer;
        curPos = readStringFromLine(curPos, typeBuffer, 2);
        curPos = advancePastSpaces(curPos);
        curPos = readFloatFromLine(curPos, &(dataBuffer[0]));
        curPos = advancePastSpaces(curPos);
        curPos = readFloatFromLine(curPos, &(dataBuffer[1]));
        curPos = advancePastSpaces(curPos);
        readFloatFromLine(curPos, &(dataBuffer[2]));

        if (strncmp(typeBuffer, "v", 1) == 0) {
            appendVertexData(&posList, typeBuffer, dataBuffer, 3);
        } else if (strncmp(typeBuffer, "vn", 2) == 0) {
            appendVertexData(&normalList, typeBuffer, dataBuffer, 3);
        } else if (strncmp(typeBuffer, "vt", 2) == 0) {
            appendVertexData(&texCoordList, typeBuffer, dataBuffer, 2);
        }
    }

    deleteVertexDataList(&texCoordList);
    deleteVertexDataList(&normalList);
    deleteVertexDataList(&posList);

    free(dataBuffer);
    dataBuffer = NULL;
    free(typeBuffer);
    typeBuffer = NULL;
    deleteLineBuffer(&lineBuffer);
}