#ifndef PY3DENGINE_WFO_PARSER_H
#define PY3DENGINE_WFO_PARSER_H

#include <stdio.h>

struct WfoParser {
    struct ObjectListNode *_objectList;
    struct VertexListNode *_positionList;
    struct VertexListNode *_normalList;
    struct VertexListNode *_texCoordList;
};

extern void allocWfoParser(struct WfoParser **wfoParserPtr);
extern void deleteWfoParser(struct WfoParser **wfoParserPtr);

extern void parseWaveFrontFile(struct WfoParser *wfoParser, FILE *wfo);

extern unsigned long getUnIndexedVertexBufferSizeInFloats(struct WfoParser *wfoParser, const char *name);
extern void getUnIndexedVertexBuffer(struct WfoParser *wfoParser, const char *name, float *dst, unsigned long limit);

#endif
