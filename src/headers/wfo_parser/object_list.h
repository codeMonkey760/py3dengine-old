#ifndef PY3DENGINE_OBJECT_LIST_H
#define PY3DENGINE_OBJECT_LIST_H

struct ObjectListNode {
    struct ObjectListNode *next;
    struct FaceListNode *faceList;
    struct String *name;
};

extern void allocObjectListNode(struct ObjectListNode **objectListNodePtr);
extern void deleteObjectListNode(struct ObjectListNode **objectListNodePtr);
extern void setObjectListNodeName(struct ObjectListNode *objectList, char *newName);

#endif
