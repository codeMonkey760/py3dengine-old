#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "logger.h"
#include "wfo_parser/vertex_data_list.h"
#include "wfo_parser/object_list.h"
#include "wfo_parser/wfo_parser.h"
#include "resource_manager.h"
#include "material.h"

#define LINE_BUFFER_SIZE_IN_ELEMENTS 256
#define TYPE_BUFFER_SIZE_IN_ELEMENTS 16
#define NAME_BUFFER_SIZE_IN_ELEMENTS 64
#define INDEX_BUFFER_SIZE_IN_ELEMENTS 9

static void clearCharBuffer(char *lineBuffer, int sizeInElements) {
    memset(lineBuffer, 0, sizeInElements * sizeof(char));
}

static void clearIndexBuffer(int *indexBuffer) {
    memset(indexBuffer, 0, INDEX_BUFFER_SIZE_IN_ELEMENTS * sizeof(int));
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

static void flattenVertexDataList(struct VertexListNode *vertexDataList, size_t elementCount, size_t elementSizeInFloats, float **dst) {
    if (vertexDataList == NULL || elementCount == 0 || elementSizeInFloats == 0 || elementSizeInFloats > 3 || dst == NULL || (*dst) != NULL) return;

    debug_log("Allocating %d * %d * %d = %d bytes for vertex data buffer", elementCount, elementSizeInFloats, sizeof(float), elementCount * elementSizeInFloats * sizeof(float));
    float *buffer = calloc(elementCount * elementSizeInFloats, sizeof(float));
    if (buffer == NULL) return;

    struct VertexListNode *curNode = vertexDataList;
    int count = 0;
    while (curNode != NULL || count < elementCount) {
        if (elementSizeInFloats != curNode->data_len) {
            // TODO: panic? At least log or something
            free(buffer);
            buffer = NULL;

            return;
        }

        for (int i = 0; i < elementSizeInFloats; ++i) {
            buffer[(count*elementSizeInFloats)+i] = curNode->data[i];
        }

        count++;
        curNode = curNode->next;
    }

    (*dst) = buffer;
    buffer = NULL;
}

static float getVertexDataElement(float *vertexDataBuffer, int index, size_t bufferSize) {
    if (vertexDataBuffer == NULL || index < 0 || index >= bufferSize || bufferSize == 0) return 0.0f;

    return vertexDataBuffer[index];
}

static int getIndexBufferElement (int *indexBuffer, int index, size_t bufferSize) {
    if (indexBuffer == NULL || index < 0 || index >= bufferSize || bufferSize == 0) return -1;

    return indexBuffer[index];
}

void allocWfoParser(struct WfoParser **wfoParserPtr) {
    if (wfoParserPtr == NULL || (*wfoParserPtr) != NULL) return;

    struct WfoParser *wfoParser = calloc(1, sizeof(struct WfoParser));
    if (wfoParser == NULL) return;

    wfoParser->_posBuffer = NULL;
    wfoParser->_posBufferSize = 0;
    wfoParser->_normalBuffer = NULL;
    wfoParser->_normalBufferSize = 0;
    wfoParser->_texCoordBuffer = NULL;
    wfoParser->_texCoordBuffSize = 0;

    wfoParser->_objectList = NULL;

    (*wfoParserPtr) = wfoParser;
    wfoParser = NULL;
}

void deleteWfoParser(struct WfoParser **wfoParserPtr) {
    if (wfoParserPtr == NULL || (*wfoParserPtr) == NULL) return;

    struct WfoParser *wfoParser = (*wfoParserPtr);
    deleteObjectListNode(&wfoParser->_objectList);

    if (wfoParser->_posBuffer != NULL) {
        free(wfoParser->_posBuffer);
        wfoParser->_posBuffer = NULL;
    }
    wfoParser->_posBufferSize = 0;
    if (wfoParser->_normalBuffer != NULL) {
        free(wfoParser->_normalBuffer);
        wfoParser->_normalBuffer = NULL;
    }
    wfoParser->_normalBufferSize = 0;
    if (wfoParser->_texCoordBuffer != NULL) {
        free(wfoParser->_texCoordBuffer);
        wfoParser->_texCoordBuffer = NULL;
    }
    wfoParser->_texCoordBuffSize = 0;

    free(wfoParser);
    wfoParser = NULL;
    (*wfoParserPtr) = NULL;
}

void parseWaveFrontFile(struct WfoParser *wfoParser, FILE *wfo) {
    if (wfoParser == NULL || wfo == NULL) return;

    char lineBuffer[LINE_BUFFER_SIZE_IN_ELEMENTS+1];
    char *curPos = NULL;
    char typeBuffer[TYPE_BUFFER_SIZE_IN_ELEMENTS+1];
    char nameBuffer[NAME_BUFFER_SIZE_IN_ELEMENTS+1];
    float dataBuffer[3];
    int indexBuffer[INDEX_BUFFER_SIZE_IN_ELEMENTS];
    int lineNumber = 0;
    size_t posCount = 0, normalCount = 0, texCoordCount = 0;

    struct VertexListNode *posList = NULL, *normalList = NULL, *texCoordList = NULL;
    struct ObjectListNode *objectList = NULL;

    clearCharBuffer(lineBuffer, LINE_BUFFER_SIZE_IN_ELEMENTS+1);
    curPos = lineBuffer;
    clearCharBuffer(nameBuffer, NAME_BUFFER_SIZE_IN_ELEMENTS+1);
    strncpy(nameBuffer, "None", NAME_BUFFER_SIZE_IN_ELEMENTS);

    while(fgets(lineBuffer, LINE_BUFFER_SIZE_IN_ELEMENTS, wfo) != NULL) {
        lineNumber++;
        clearCharBuffer(typeBuffer, TYPE_BUFFER_SIZE_IN_ELEMENTS+1);
        curPos = readStringFromLine(curPos, typeBuffer, TYPE_BUFFER_SIZE_IN_ELEMENTS);
        curPos = advancePastSpaces(curPos);

        if (strncmp(typeBuffer, "v", TYPE_BUFFER_SIZE_IN_ELEMENTS) == 0) {
            Vec3Identity(dataBuffer);
            readFloatsFromLine(curPos, dataBuffer, 3);
            appendVertexData(&posList, typeBuffer, dataBuffer, 3);
            posCount++;
        } else if (strncmp(typeBuffer, "vn", TYPE_BUFFER_SIZE_IN_ELEMENTS) == 0) {
            Vec3Identity(dataBuffer);
            readFloatsFromLine(curPos, dataBuffer, 3);
            appendVertexData(&normalList, typeBuffer, dataBuffer, 3);
            normalCount++;
        } else if (strncmp(typeBuffer, "vt", TYPE_BUFFER_SIZE_IN_ELEMENTS) == 0) {
            Vec3Identity(dataBuffer);
            readFloatsFromLine(curPos, dataBuffer, 2);
            appendVertexData(&texCoordList, typeBuffer, dataBuffer, 2);
            texCoordCount++;
        } else if (strncmp(typeBuffer, "o", TYPE_BUFFER_SIZE_IN_ELEMENTS) == 0) {
            clearCharBuffer(nameBuffer, NAME_BUFFER_SIZE_IN_ELEMENTS + 1);
            readStringFromLine(curPos, nameBuffer, NAME_BUFFER_SIZE_IN_ELEMENTS);
        } else if (strncmp(typeBuffer, "f", TYPE_BUFFER_SIZE_IN_ELEMENTS) == 0) {
            clearIndexBuffer(indexBuffer);
            readIndicesFromLine(curPos, indexBuffer);
            appendFaceToObjectList(&objectList, nameBuffer, indexBuffer);
        } else {
            debug_log("WFO Parser: Ignoring line #%d: unsupported type '%s'", lineNumber, typeBuffer);
        }

        clearCharBuffer(lineBuffer, LINE_BUFFER_SIZE_IN_ELEMENTS+1);
        curPos = lineBuffer;
    }

    debug_log("Found %d positions, %d normals, %d texture coordinates", posCount, normalCount, texCoordCount);

    flattenVertexDataList(posList, posCount, 3, &wfoParser->_posBuffer);
    wfoParser->_posBufferSize = posCount * 3;
    flattenVertexDataList(normalList, normalCount, 3, &wfoParser->_normalBuffer);
    wfoParser->_normalBufferSize = normalCount * 3;
    flattenVertexDataList(texCoordList, texCoordCount, 2, &wfoParser->_texCoordBuffer);
    wfoParser->_texCoordBuffSize = texCoordCount * 2;
    flattenObjectList(objectList);

    wfoParser->_objectList = objectList;

    deleteVertexDataList(&posList);
    deleteVertexDataList(&normalList);
    deleteVertexDataList(&texCoordList);
}

void parseMaterialFile(struct ResourceManager *manager, FILE *mtl) {
    if (manager == NULL || mtl == NULL) return;

    char lineBuffer[LINE_BUFFER_SIZE_IN_ELEMENTS+1];
    char *curPos;
    char typeBuffer[TYPE_BUFFER_SIZE_IN_ELEMENTS+1];
    char nameBuffer[NAME_BUFFER_SIZE_IN_ELEMENTS+1];
    float dataBuffer[3] = {0.0f};
    int lineNumber = 0;

    clearCharBuffer(lineBuffer, LINE_BUFFER_SIZE_IN_ELEMENTS+1);
    curPos = lineBuffer;

    struct Material *curMaterial = NULL;
    while (fgets(lineBuffer, LINE_BUFFER_SIZE_IN_ELEMENTS, mtl)) {
        lineNumber++;
        clearCharBuffer(typeBuffer, TYPE_BUFFER_SIZE_IN_ELEMENTS+1);
        curPos = readStringFromLine(curPos, typeBuffer, TYPE_BUFFER_SIZE_IN_ELEMENTS);
        curPos = advancePastSpaces(curPos);

        if (strncmp(typeBuffer, "newmtl", TYPE_BUFFER_SIZE_IN_ELEMENTS) == 0) {
            clearCharBuffer(nameBuffer, NAME_BUFFER_SIZE_IN_ELEMENTS+1);
            curPos = readStringFromLine(curPos, nameBuffer, NAME_BUFFER_SIZE_IN_ELEMENTS);
            if (curMaterial != NULL) {
                storeMaterial(manager, curMaterial);
                curMaterial = NULL;
            }
            allocMaterial(&curMaterial);
            setMaterialName(curMaterial, nameBuffer);
        } else if (strncmp(typeBuffer, "Kd", TYPE_BUFFER_SIZE_IN_ELEMENTS) == 0) {
            if (curMaterial == NULL) {
                error_log("%s", "[WfoParser]: Trying to write diffuse color into NULL material.");
                continue;
            }
            readFloatsFromLine(curPos, dataBuffer, 3);
            setMaterialDiffuseColor(curMaterial, dataBuffer);
        } else {
            debug_log("[WfoParser]: Ignoring line #%d, unsupported type %s", lineNumber, typeBuffer);
        }
    }

    if (curMaterial != NULL) {
        storeMaterial(manager, curMaterial);
        curMaterial = NULL;
    }
}

unsigned long getUnIndexedVertexBufferSizeInFloats(struct WfoParser *wfoParser, const char *name) {
    if (wfoParser == NULL || name == NULL) return 0;

    struct ObjectListNode *curNode = wfoParser->_objectList;

    while (curNode != NULL) {
        if (strncmp(curNode->name, name, NAME_BUFFER_SIZE_IN_ELEMENTS) == 0) {
            break;
        }

        curNode = curNode->next;
    }

    // This will handle curNode == NULL
    // TODO: this is a nasty hack
    return getIndexBufferSize(curNode) / 3 * 8;
}

void getUnIndexedVertexBuffer(struct WfoParser *wfoParser, const char *name, float *dst, size_t limit) {
    if (wfoParser == NULL || name == NULL || dst == NULL || limit == 0) return;

    struct ObjectListNode *curNode = wfoParser->_objectList;

    while(curNode != NULL) {
        if (strncmp(curNode->name, name, NAME_BUFFER_SIZE_IN_ELEMENTS) == 0) {
            break;
        }

        curNode = curNode->next;
    }
    if (curNode == NULL) return;

    int *indexBuffer = getIndexBuffer(curNode);
    size_t indexBufferSize = getIndexBufferSize(curNode);
    if (indexBuffer == NULL || indexBufferSize == 0) return;
    int elementCount = 0;

    // TODO: the end clause on this loop isn't correct if the buffer isn't mod 3 aligned
    for (int i = 0; i < indexBufferSize; i+=3) {
        int posIndex = getIndexBufferElement(indexBuffer, i + 0, indexBufferSize) - 1;
        // yes this is correct, wfo stores vertex indices in ptn format and I want them in pnt format
        int normalIndex = getIndexBufferElement(indexBuffer, i + 2, indexBufferSize) - 1;
        int texCoordIndex = getIndexBufferElement(indexBuffer, i + 1, indexBufferSize) - 1;

        dst[elementCount*8+0] = getVertexDataElement(wfoParser->_posBuffer, (posIndex * 3) + 0, wfoParser->_posBufferSize);
        dst[elementCount*8+1] = getVertexDataElement(wfoParser->_posBuffer, (posIndex * 3) + 1, wfoParser->_posBufferSize);
        dst[elementCount*8+2] = getVertexDataElement(wfoParser->_posBuffer, (posIndex * 3) + 2, wfoParser->_posBufferSize);

        dst[elementCount*8+3] = getVertexDataElement(wfoParser->_normalBuffer, (normalIndex * 3) + 0, wfoParser->_normalBufferSize);
        dst[elementCount*8+4] = getVertexDataElement(wfoParser->_normalBuffer, (normalIndex * 3) + 1, wfoParser->_normalBufferSize);
        dst[elementCount*8+5] = getVertexDataElement(wfoParser->_normalBuffer, (normalIndex * 3) + 2, wfoParser->_normalBufferSize);

        dst[elementCount*8+6] = getVertexDataElement(wfoParser->_texCoordBuffer, (texCoordIndex * 2) + 0, wfoParser->_texCoordBuffSize);
        dst[elementCount*8+7] = getVertexDataElement(wfoParser->_texCoordBuffer, (texCoordIndex * 2) + 1, wfoParser->_texCoordBuffSize);

        elementCount++;
    }
}