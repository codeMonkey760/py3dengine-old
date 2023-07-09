#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "logger.h"
#include "custom_string.h"
#include "wfo_parser/vertex_data_list.h"
#include "wfo_parser/object_list.h"
#include "wfo_parser/wfo_parser.h"
#include "python/py3dresourcemanager.h"
#include "resources/material.h"
#include "resources/texture.h"
#include "resources/model.h"

#define LINE_BUFFER_SIZE_IN_ELEMENTS 256
#define TYPE_BUFFER_SIZE_IN_ELEMENTS 16
#define NAME_BUFFER_SIZE_IN_ELEMENTS 64
#define INDEX_BUFFER_SIZE_IN_ELEMENTS 9
#define FILE_NAME_BUFFER_SIZE_IN_ELEMENTS 256

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

static float getVertexDataElement(float *vertexDataBuffer, int index, size_t bufferSize) {
    if (vertexDataBuffer == NULL || bufferSize == 0) return 0.0f;

    if (index < 0 || index >= bufferSize) {
        critical_log("Bad vertex data read while generating vertex buffer. VB will be corrupt. Read was \"%d\" / \"%d\"", index, bufferSize);
        return 0.0f;
    }

    return vertexDataBuffer[index];
}

static int getIndexBufferElement (int *indexBuffer, int index, size_t bufferSize) {
    if (indexBuffer == NULL || bufferSize == 0) return 1;

    if (index < 0 || index >= bufferSize) {
        critical_log("Bad index buffer read while generating vertex buffer. VB will be corrupt. Read was \"%d\" / \"%d\"", index, bufferSize);
        return 1;
    }

    return indexBuffer[index];
}

static void generateVertexBuffer(
    struct VertexPNT **dst, size_t *dstSize,
    float *posBuffer, size_t posSize,
    float *normBuffer, size_t normSize,
    float *tcBuffer, size_t tcSize,
    int *indexBuffer, size_t indexBufferSize
) {
    if (
        dst == NULL || (*dst) != NULL || dstSize == NULL ||
        posBuffer == NULL || posSize == 0 ||
        normBuffer == NULL || normSize == 0 ||
        tcBuffer == NULL || tcSize == 0 ||
        indexBuffer == NULL || indexBufferSize == 0
    ) return;

    if (indexBufferSize % 3 != 0) {
        critical_log("[WfoParser]: generateVertexBuffer sanity check failed. Index buffer is not mod 3 aligned");
        return;
    }

    size_t vbSizeInVertices = indexBufferSize / 3;
    struct VertexPNT *newVB = calloc(vbSizeInVertices, sizeof(struct VertexPNT));

    for (size_t i = 0; i < vbSizeInVertices; ++i) {
        // wfo indices start at 1 not 0 so subtract 1
        int posIndex = getIndexBufferElement(indexBuffer, i * 3 + 0, indexBufferSize) -1;
        // yes, these offsets are correct, wfo stores vertices as PTN format, I want them in PNT format
        int normIndex = getIndexBufferElement(indexBuffer, i * 3 + 2, indexBufferSize) -1;
        int texCoordIndex = getIndexBufferElement(indexBuffer, i * 3 + 1, indexBufferSize) -1;

        newVB[i].position[0] = getVertexDataElement(posBuffer, (posIndex) * 3 + 0, posSize);
        newVB[i].position[1] = getVertexDataElement(posBuffer, (posIndex) * 3 + 1, posSize);
        newVB[i].position[2] = getVertexDataElement(posBuffer, (posIndex) * 3 + 2, posSize);

        newVB[i].normal[0] = getVertexDataElement(normBuffer, (normIndex) * 3 + 0, normSize);
        newVB[i].normal[1] = getVertexDataElement(normBuffer, (normIndex) * 3 + 1, normSize);
        newVB[i].normal[2] = getVertexDataElement(normBuffer, (normIndex) * 3 + 2, normSize);

        newVB[i].texCoord[0] = getVertexDataElement(tcBuffer, (texCoordIndex) * 2 + 0, tcSize);
        newVB[i].texCoord[1] = getVertexDataElement(tcBuffer, (texCoordIndex) * 2 + 1, tcSize);
    }

    (*dst) = newVB;
    newVB = NULL;
    (*dstSize) = vbSizeInVertices;
}

void importWaveFrontFile(struct Py3dResourceManager *manager, const char *filePath) {
    if (Py3dResourceManager_Check((PyObject *) manager) != 1 || filePath == NULL) return;

    FILE *wfoFile = fopen(filePath, "r");
    if (wfoFile == NULL) {
        error_log("[WfoParser]: Could not open \"%s\" for reading", filePath);
        return;
    }

    parseWaveFrontFile(manager, wfoFile);
    fclose(wfoFile);
    wfoFile = NULL;
}

void parseWaveFrontFile(struct Py3dResourceManager *manager, FILE *wfo) {
    if (Py3dResourceManager_Check((PyObject *) manager) != 1 || wfo == NULL) return;

    char lineBuffer[LINE_BUFFER_SIZE_IN_ELEMENTS+1];
    char *curPos = NULL;
    char typeBuffer[TYPE_BUFFER_SIZE_IN_ELEMENTS+1];
    char nameBuffer[NAME_BUFFER_SIZE_IN_ELEMENTS+1];
    float dataBuffer[3];
    int indexBuffer[INDEX_BUFFER_SIZE_IN_ELEMENTS];
    int lineNumber = 0;
    size_t posCount = 0, normalCount = 0, texCoordCount = 0;

    struct VectorListNode *posList = NULL, *normalList = NULL, *texCoordList = NULL;
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
            appendVector3(&posList, dataBuffer[0], dataBuffer[1], dataBuffer[2]);
            posCount++;
        } else if (strncmp(typeBuffer, "vn", TYPE_BUFFER_SIZE_IN_ELEMENTS) == 0) {
            Vec3Identity(dataBuffer);
            readFloatsFromLine(curPos, dataBuffer, 3);
            appendVector3(&normalList, dataBuffer[0], dataBuffer[1], dataBuffer[2]);
            normalCount++;
        } else if (strncmp(typeBuffer, "vt", TYPE_BUFFER_SIZE_IN_ELEMENTS) == 0) {
            Vec3Identity(dataBuffer);
            readFloatsFromLine(curPos, dataBuffer, 2);
            appendVector2(&texCoordList, dataBuffer[0], dataBuffer[1]);
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

    float *positionFloatBuffer = NULL, *normalFloatBuffer = NULL, *texCoordFloatBuffer = NULL;
    size_t positionBufferSize = 0, normalBufferSize = 0, texCoordBufferSize = 0;

    flattenVectorList(posList, &positionFloatBuffer, &positionBufferSize);
    flattenVectorList(normalList, &normalFloatBuffer, &normalBufferSize);
    flattenVectorList(texCoordList, &texCoordFloatBuffer, &texCoordBufferSize);
    flattenObjectList(objectList);

    deleteVectorList(&posList);
    deleteVectorList(&normalList);
    deleteVectorList(&texCoordList);

    struct ObjectListNode *curNode = objectList;
    while (curNode != NULL) {
        struct VertexPNT *vb = NULL;
        size_t vbSizeInVertices;
        generateVertexBuffer(
            &vb, &vbSizeInVertices,
            positionFloatBuffer, positionBufferSize,
            normalFloatBuffer, normalBufferSize,
            texCoordFloatBuffer, texCoordBufferSize,
            curNode->indexBuffer, curNode->indexBufferSize
        );
        if (vb == NULL) {
            curNode = curNode->next;
            continue;
        }

        struct Model *newModel = NULL;
        allocModel(&newModel);
        if (newModel == NULL) {
            curNode = curNode->next;
            continue;
        }
        setResourceName((struct BaseResource *) newModel, curNode->name);
        setModelPNTBuffer(newModel, vb, vbSizeInVertices);

        free(vb);
        vb = NULL;

        trace_log("[WfoParser]: Storing material named \"%s\"", curNode->name);
        Py3dResourceManager_StoreResource(manager, (struct BaseResource *) newModel);
        newModel = NULL;

        curNode = curNode->next;
    }

    free(positionFloatBuffer);
    positionFloatBuffer = NULL;
    positionBufferSize = 0;

    free(normalFloatBuffer);
    normalFloatBuffer = NULL;
    normalBufferSize = 0;

    free(texCoordFloatBuffer);
    texCoordFloatBuffer = NULL;
    texCoordBufferSize = 0;

    deleteObjectListNode(&objectList);
}

void importMaterialFile(struct Py3dResourceManager *manager, const char *filePath) {
    if (Py3dResourceManager_Check((PyObject *) manager) != 1 || filePath == NULL) return;

    FILE *mtlFile = fopen(filePath, "r");
    if (mtlFile == NULL) {
        error_log("[WfoParser]: Could not open \"%s\" for reading", filePath);
        return;
    }

    parseMaterialFile(manager, mtlFile);
    fclose(mtlFile);
    mtlFile = NULL;
}

void parseMaterialFile(struct Py3dResourceManager *manager, FILE *mtl) {
    if (Py3dResourceManager_Check((PyObject *) manager) != 1 || mtl == NULL) return;

    char lineBuffer[LINE_BUFFER_SIZE_IN_ELEMENTS+1];
    char *curPos;
    char typeBuffer[TYPE_BUFFER_SIZE_IN_ELEMENTS+1];
    char nameBuffer[NAME_BUFFER_SIZE_IN_ELEMENTS+1];
    char fileNameBuffer[FILE_NAME_BUFFER_SIZE_IN_ELEMENTS+1];
    float dataBuffer[3] = {0.0f};
    int lineNumber = 0;

    clearCharBuffer(lineBuffer, LINE_BUFFER_SIZE_IN_ELEMENTS+1);
    curPos = lineBuffer;

    struct BaseResource *curMaterial = NULL;
    while (fgets(lineBuffer, LINE_BUFFER_SIZE_IN_ELEMENTS, mtl)) {
        lineNumber++;
        if (strnlen(lineBuffer, LINE_BUFFER_SIZE_IN_ELEMENTS) < 2) {
            continue;
        }
        clearCharBuffer(typeBuffer, TYPE_BUFFER_SIZE_IN_ELEMENTS+1);
        curPos = readStringFromLine(curPos, typeBuffer, TYPE_BUFFER_SIZE_IN_ELEMENTS);
        curPos = advancePastSpaces(curPos);

        if (strncmp(typeBuffer, "newmtl", TYPE_BUFFER_SIZE_IN_ELEMENTS) == 0) {
            clearCharBuffer(nameBuffer, NAME_BUFFER_SIZE_IN_ELEMENTS+1);
            readStringFromLine(curPos, nameBuffer, NAME_BUFFER_SIZE_IN_ELEMENTS);
            if (curMaterial != NULL) {
                trace_log("[WfoParser]: Storing material named \"%s\"", getChars(getResourceName(curMaterial)));
                Py3dResourceManager_StoreResource(manager, curMaterial);
                curMaterial = NULL;
            }
            trace_log("[WfoParser]: Allocating new material named \"%s\"", nameBuffer);
            allocMaterial((struct Material **) &curMaterial);
            setResourceName(curMaterial, nameBuffer);
        } else if (strncmp(typeBuffer, "Kd", TYPE_BUFFER_SIZE_IN_ELEMENTS) == 0) {
            if (curMaterial == NULL) {
                error_log("%s", "[WfoParser]: Trying to write diffuse color into NULL material.");
                continue;
            }
            readFloatsFromLine(curPos, dataBuffer, 3);
            setMaterialDiffuseColor((struct Material *) curMaterial, dataBuffer);
            trace_log(
                "[WfoParser]: Writing (%.2f, %.2f, %.2f) as diffuse color to material named \"%s\"",
                dataBuffer[0],
                dataBuffer[1],
                dataBuffer[2],
                getChars(getResourceName(curMaterial))
            );
        } else if (strncmp(typeBuffer, "map_Kd", TYPE_BUFFER_SIZE_IN_ELEMENTS) == 0) {
            if (curMaterial == NULL) {
                error_log("%s", "[WfoParser]: Trying to write diffuse texture into NULL material.");
                continue;
            }
            clearCharBuffer(fileNameBuffer, FILE_NAME_BUFFER_SIZE_IN_ELEMENTS+1);
            readStringFromLine(curPos, fileNameBuffer, FILE_NAME_BUFFER_SIZE_IN_ELEMENTS);

            struct BaseResource *diffuse_map = Py3dResourceManager_GetResource(manager, fileNameBuffer);
            if (!isResourceTypeTexture(diffuse_map)) {
                error_log("[WfoParser]: Material specifies non existent texture named \"%s\" as a diffuse map", fileNameBuffer);
            } else {
                trace_log("[WfoParser]: Setting \"%s\" as a diffuse map", fileNameBuffer);
                setMaterialDiffuseMap((struct Material *) curMaterial, (struct Texture *) diffuse_map);
                diffuse_map = NULL;
            }

        } else {
            debug_log("[WfoParser]: Ignoring line #%d, unsupported type %s", lineNumber, typeBuffer);
        }

        clearCharBuffer(lineBuffer, LINE_BUFFER_SIZE_IN_ELEMENTS+1);
        curPos = lineBuffer;
    }

    if (curMaterial != NULL) {
        trace_log("[WfoParser]: Storing material named \"%s\"", getChars(getResourceName(curMaterial)));
        Py3dResourceManager_StoreResource(manager, curMaterial);
        curMaterial = NULL;
    }
}
