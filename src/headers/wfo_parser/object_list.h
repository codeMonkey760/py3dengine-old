#ifndef PY3DENGINE_OBJECT_LIST_H
#define PY3DENGINE_OBJECT_LIST_H

struct ObjectListNode {
    struct ObjectListNode *next;
    struct FaceListNode *faceList;
    char *name;
    int *indexBuffer;
    size_t indexBufferSize;
};

extern void appendFaceToObjectList(struct ObjectListNode **objectListPtr, char *name, int indexBuffer[9]);
extern void deleteObjectListNode(struct ObjectListNode **objectListNodePtr);

extern unsigned long getFaceCount(struct ObjectListNode *objectList);
extern void flattenObjectList(struct ObjectListNode *objectList);
extern void getIndexBuffer(struct ObjectListNode *objectListNode, int **indexBufferPtr, size_t *sizePtr);

#endif
