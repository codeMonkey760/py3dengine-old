#ifndef PY3DENGINE_WFO_PARSER_H
#define PY3DENGINE_WFO_PARSER_H

#include <stdio.h>

struct ResourceManager;

struct WfoParser {
    struct ObjectListNode *_objectList;

    float *_posBuffer;
    size_t _posBufferSize;
    float *_normalBuffer;
    size_t _normalBufferSize;
    float *_texCoordBuffer;
    size_t _texCoordBuffSize;
};

extern void allocWfoParser(struct WfoParser **wfoParserPtr);
extern void deleteWfoParser(struct WfoParser **wfoParserPtr);

extern void parseWaveFrontFile(struct WfoParser *wfoParser, FILE *wfo);
extern void parseMaterialFile(struct ResourceManager *manager, FILE *mtl);

extern unsigned long getUnIndexedVertexBufferSizeInFloats(struct WfoParser *wfoParser, const char *name);
extern void getUnIndexedVertexBuffer(struct WfoParser *wfoParser, const char *name, float *dst, size_t limit);

#endif
