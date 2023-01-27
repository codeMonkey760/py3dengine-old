#ifndef PY3DENGINE_VERTEX_DATA_LIST_H
#define PY3DENGINE_VERTEX_DATA_LIST_H

#include <stdio.h>

struct Vec2ListNode {
    struct Vec2ListNode *next;
    float elements[2];
};

extern void appendVec2(struct Vec2ListNode **vec2ListNodePtr, float x, float y);
extern void deleteVec2List(struct Vec2ListNode **vec2ListNodePtr);
extern void flattenVec2List(struct Vec2ListNode, float *dst, size_t *dstSize);

struct Vec3ListNode {
    struct Vec3ListNode *next;
    float elements[3];
};

extern void appendVec3(struct Vec3ListNode **vec3ListNodePtr, float x, float y, float z);
extern void deleteVec3List(struct Vec3ListNode **vec3ListNodePtr);
extern void flattenVec3List(struct Vec3ListNode, float *dst, size_t *dstSize);

#endif
