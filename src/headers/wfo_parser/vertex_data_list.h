#ifndef PY3DENGINE_VERTEX_DATA_LIST_H
#define PY3DENGINE_VERTEX_DATA_LIST_H

struct VertexListNode {
    struct VertexListNode *next;
    float data[3];
    char type[3];
    int data_len; //number of floats used from data array
};

extern void appendVertexData(struct VertexListNode **vertexDataListPtr, char *type, float *src, int size);
extern void deleteVertexDataList(struct VertexListNode **vertexDataListPtr);

#endif
