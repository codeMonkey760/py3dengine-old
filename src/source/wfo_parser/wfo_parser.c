#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "logger.h"
#include "wfo_parser/vertex_data_list.h"
#include "wfo_parser/object_list.h"
#include "wfo_parser/wfo_parser.h"

static const int line_buffer_size_in_elements = 256;
static const int type_buffer_size_in_elements = 16;
static const int name_buffer_size_in_elements = 64;
static const int index_buffer_size_in_elements = 9;

static void clearCharBuffer(char *lineBuffer, int sizeInElements) {
    memset(lineBuffer, 0, sizeInElements * sizeof(char));
}

static void clearIndexBuffer(int *indexBuffer) {
    memset(indexBuffer, 0, index_buffer_size_in_elements * sizeof(int));
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
        dst++;
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

static char* readIntFromLine(char *curPos, int *dst) {
    if (curPos == NULL || (*curPos) == 0 || dst == NULL) return curPos;

    if ( (*curPos) == '/' ) {
        (*dst) = -1;

        return curPos;
    }

    char charBuffer[33];
    char *charBufferCursor = charBuffer;
    memset(charBuffer, 0, 33 * sizeof(char));

    for (int i = 0; i < 32; ++i) {
        char curChar = (*curPos);

        if (!isdigit(curChar)) {
            break;
        }

        (*charBufferCursor) = curChar;
        charBufferCursor++;
        curPos++;
    }

    (*dst) = (int) strtol(charBuffer, NULL, 10);

    return curPos;
}

static char* consumeIndexSeparator(char *curPos) {
    if (curPos == NULL) return curPos;

    if ( (*curPos) == '/') {
        return curPos+1;
    } else {
        return curPos;
    }
}

static char* readIndicesFromLine(char *curPos, int *indexBuffer) {
    if (curPos == NULL || (*curPos) == 0 || indexBuffer == NULL) return curPos;

    for (int i = 0; i < 3; ++i) {
        curPos = readIntFromLine(curPos, &(indexBuffer[i*3+0]));
        curPos = consumeIndexSeparator(curPos);
        curPos = readIntFromLine(curPos, &(indexBuffer[i*3+1]));
        curPos = consumeIndexSeparator(curPos);
        curPos = readIntFromLine(curPos, &(indexBuffer[i*3+2]));
        curPos = advancePastSpaces(curPos);
    }

    return curPos;
}

void parseWaveFrontFile(FILE *wfo, struct Model **modelPtr) {
    if (wfo == NULL) return;

    char lineBuffer[line_buffer_size_in_elements+1];
    char *curPos = NULL;
    char typeBuffer[type_buffer_size_in_elements+1];
    char nameBuffer[name_buffer_size_in_elements+1];
    float dataBuffer[3];
    int indexBuffer[index_buffer_size_in_elements];
    int lineNumber = 0;
    int posCount = 0, normalCount = 0, texCoordCount = 0;

    struct VertexListNode *posList = NULL, *normalList = NULL, *texCoordList = NULL;
    struct ObjectListNode *objectList = NULL;

    clearCharBuffer(lineBuffer, line_buffer_size_in_elements+1);
    curPos = lineBuffer;
    clearCharBuffer(nameBuffer, name_buffer_size_in_elements+1);
    strncpy(nameBuffer, "None", name_buffer_size_in_elements);

    while(fgets(lineBuffer, line_buffer_size_in_elements, wfo) != NULL) {
        lineNumber++;
        clearCharBuffer(typeBuffer, type_buffer_size_in_elements+1);
        curPos = readStringFromLine(curPos, typeBuffer, name_buffer_size_in_elements);
        curPos = advancePastSpaces(curPos);

        if (strncmp(typeBuffer, "v", type_buffer_size_in_elements) == 0) {
            Vec3Identity(dataBuffer);
            readFloatsFromLine(curPos, dataBuffer, 3);
            appendVertexData(&posList, typeBuffer, dataBuffer, 3);
            posCount++;
        } else if (strncmp(typeBuffer, "vn", type_buffer_size_in_elements) == 0) {
            Vec3Identity(dataBuffer);
            readFloatsFromLine(curPos, dataBuffer, 3);
            appendVertexData(&normalList, typeBuffer, dataBuffer, 3);
            normalCount++;
        } else if (strncmp(typeBuffer, "vt", type_buffer_size_in_elements) == 0) {
            Vec3Identity(dataBuffer);
            readFloatsFromLine(curPos, dataBuffer, 2);
            appendVertexData(&texCoordList, typeBuffer, dataBuffer, 2);
            texCoordCount++;
        } else if (strncmp(typeBuffer, "o", type_buffer_size_in_elements) == 0) {
            clearCharBuffer(nameBuffer, name_buffer_size_in_elements + 1);
            readStringFromLine(curPos, nameBuffer, name_buffer_size_in_elements);
        } else if (strncmp(typeBuffer, "f", type_buffer_size_in_elements) == 0) {
            clearIndexBuffer(indexBuffer);
            readIndicesFromLine(curPos, indexBuffer);
            appendFaceToObjectList(&objectList, nameBuffer, indexBuffer);
        } else {
            debug_log("WFO Parser: Ignoring line #%d: unsupported type '%s'", lineNumber, typeBuffer);
        }

        clearCharBuffer(lineBuffer, line_buffer_size_in_elements+1);
        curPos = lineBuffer;
    }

    debug_log("Found %d positions, %d normals, %d texture coordinates", posCount, normalCount, texCoordCount);

    deleteObjectListNode(&objectList);
    deleteVertexDataList(&texCoordList);
    deleteVertexDataList(&normalList);
    deleteVertexDataList(&posList);
}