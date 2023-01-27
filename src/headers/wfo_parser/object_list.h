#ifndef PY3DENGINE_OBJECT_LIST_H
#define PY3DENGINE_OBJECT_LIST_H

struct FaceListNode;

struct ObjectListNode {
    struct ObjectListNode *next;
    struct FaceListNode *faceList;
    char *name;
    int *indexBuffer;
    size_t indexBufferSize;
};

extern void appendFaceToObjectList(struct ObjectListNode **objectListPtr, char *name, int indexBuffer[9]);
extern void deleteObjectListNode(struct ObjectListNode **objectListNodePtr);

extern void flattenObjectList(struct ObjectListNode *objectList);
extern void getIndexBuffer(struct ObjectListNode *objectListNode, const char *name, int **dst, size_t *dstSize);

#endif
