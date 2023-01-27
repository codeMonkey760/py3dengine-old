#ifndef PY3DENGINE_VERTEX_DATA_LIST_H
#define PY3DENGINE_VERTEX_DATA_LIST_H

struct VectorListNode {
    struct VectorListNode *next;
    int size;
    float elements[3];
};

extern void appendVector2(struct VectorListNode **vecListNodePtr, float x, float y);
extern void appendVector3(struct VectorListNode **vecListNodePtr, float x, float y, float z);
extern void deleteVectorList(struct VectorListNode **vecListNodePtr);
extern void flattenVectorList(struct VectorListNode *vecList, float **dstPtr, size_t *dstSizePtr);

#endif
