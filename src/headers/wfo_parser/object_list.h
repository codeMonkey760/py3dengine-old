#ifndef PY3DENGINE_OBJECT_LIST_H
#define PY3DENGINE_OBJECT_LIST_H

struct ObjectListNode {
    struct ObjectListNode *next;
    struct FaceListNode *faceList;
    char *name;
};

extern void appendFaceToObjectList(struct ObjectListNode **objectListPtr, char *name, int indexBuffer[9]);
extern void deleteObjectListNode(struct ObjectListNode **objectListNodePtr);

extern unsigned long getFaceCount(struct ObjectListNode *objectList);
extern void getUnIndexedVertexBufferFromObject(struct ObjectListNode *objectList, float *dst, unsigned long limit);

#endif
