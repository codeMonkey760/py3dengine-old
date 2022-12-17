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

extern void flattenObjectList(struct ObjectListNode *objectList);
extern int *getIndexBuffer(struct ObjectListNode *objectListNode);
extern size_t getIndexBufferSize(struct ObjectListNode *objectListNode);

#endif
