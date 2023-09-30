#include <stdlib.h>

#include "physics/collision_state.h"

struct CollisionStateEntry {
    struct Py3dRigidBody *key;
    struct Py3dRigidBody *value;
    struct CollisionStateEntry *next;
};

static void allocCollisionStateEntry(struct CollisionStateEntry **entryPtr) {
    if (entryPtr == NULL || (*entryPtr) != NULL) return;

    struct CollisionStateEntry *entry = calloc(1, sizeof(struct CollisionStateEntry));
    if (entry == NULL) return;

    entry->key = NULL;
    entry->value = NULL;
    entry->next = NULL;

    (*entryPtr) = entry;
    entry = NULL;
}

static void deallocCollisionStateEntry(struct CollisionStateEntry **entryPtr) {
    if (entryPtr == NULL || (*entryPtr) == NULL) return;

    struct CollisionStateEntry *entry = (*entryPtr);

    deallocCollisionStateEntry(&entry->next);
    entry->value = NULL;
    entry->key = NULL;

    free(entry);
    entry = NULL;
    (*entryPtr) = NULL;
}

void allocCollisionState(struct CollisionState **statePtr) {
    if (statePtr == NULL || (*statePtr) != NULL) return;

    struct CollisionState *table = calloc(1, sizeof(struct CollisionState));
    if (table == NULL) return;

    table->head = NULL;

    (*statePtr) = table;
    table = NULL;
}

void deallocCollisionState(struct CollisionState **statePtr) {
    if (statePtr == NULL || (*statePtr) == NULL) return;

    struct CollisionState *table = (*statePtr);
    deallocCollisionStateEntry(&table->head);

    free(table);
    table = NULL;
    (*statePtr) = NULL;
}

void addCollisionToState(struct CollisionState *state, struct Py3dRigidBody *key, struct Py3dRigidBody *value) {
    if (state == NULL || key == NULL || value == NULL) return;

    struct CollisionStateEntry *curNode = state->head, *prevNode = NULL;
    while (curNode != NULL) {
        if (curNode->key == key && curNode->value == value) return;

        prevNode = curNode;
        curNode = curNode->next;
    }

    struct CollisionStateEntry *newEntry = NULL;
    allocCollisionStateEntry(&newEntry);
    if (newEntry == NULL) return;

    newEntry->key = key;
    newEntry->value = value;

    if (prevNode == NULL) {
        state->head = newEntry;
    } else {
        prevNode->next = newEntry;
    }
}

static int stateHasCollision(struct CollisionState *state, struct Py3dRigidBody *rb1, struct Py3dRigidBody *rb2) {
    if (state == NULL || rb1 == NULL || rb2 == NULL) return 0;

    struct CollisionStateEntry *curNode = state->head;
    while (curNode != NULL) {
        if (curNode->key == rb1 && curNode->value == rb2) {
            return 1;
        }

        curNode = curNode->next;
    }

    return 0;
}

static void allocCollisionStateDiffEntry(struct CollisionStateDiffEntry **entryPtr) {
    if (entryPtr == NULL || (*entryPtr) != NULL) return;

    struct CollisionStateDiffEntry *newEntry = calloc(1, sizeof(struct CollisionStateDiffEntry));
    if (newEntry == NULL) return;

    newEntry->rb1 = NULL;
    newEntry->rb2 = NULL;
    newEntry->isAddition = 0;
    newEntry->next = NULL;

    (*entryPtr) = newEntry;
    newEntry = NULL;
}

static void deallocCollisionStateDiffEntry(struct CollisionStateDiffEntry **entryPtr) {
    if (entryPtr == NULL || (*entryPtr) == NULL) return;

    struct CollisionStateDiffEntry *entry = (*entryPtr);
    deallocCollisionStateDiffEntry(&entry->next);
    entry->rb1 = NULL;
    entry->rb2 = NULL;

    free(entry);
    entry = NULL;
    (*entryPtr) = NULL;
}

static void addCollisionToStateDiff(struct CollisionStateDiff *diff, struct Py3dRigidBody *rb1, struct Py3dRigidBody *rb2, int isAddition) {
    if (diff == NULL || rb1 == NULL || rb2 == NULL) return;

    struct CollisionStateDiffEntry *prevNode = NULL, *curNode = diff->head;
    while (curNode != NULL) {
        if (curNode->rb1 == rb1 && curNode->rb2 == rb2 && curNode->isAddition == isAddition) return;

        prevNode = curNode;
        curNode = curNode->next;
    }

    struct CollisionStateDiffEntry *newNode = NULL;
    allocCollisionStateDiffEntry(&newNode);
    if (newNode == NULL) return;

    newNode->rb1 = rb1;
    newNode->rb2 = rb2;
    newNode->isAddition = isAddition;

    if (prevNode == NULL) {
        diff->head = newNode;
    } else {
        prevNode->next = newNode;
    }
}

extern void calculateCollisionStateDiff(struct CollisionStateDiff *diff, struct CollisionState *oldState, struct CollisionState *newState) {
    if (diff == NULL || oldState == NULL || newState == NULL) return;

    if (diff->head != NULL) return;

    struct CollisionStateEntry *curNode = oldState->head;
    while(curNode != NULL) {
        if (!stateHasCollision(newState, curNode->key, curNode->value)) {
            addCollisionToStateDiff(diff, curNode->key, curNode->value, 0);
        }

        curNode = curNode->next;
    }

    curNode = newState->head;
    while(curNode != NULL) {
        if (!stateHasCollision(oldState, curNode->key, curNode->value)) {
            addCollisionToStateDiff(diff, curNode->key, curNode->value, 1);
        }

        curNode = curNode->next;
    }
}

extern void allocCollisionStateDiff(struct CollisionStateDiff **diffPtr) {
    if (diffPtr == NULL || (*diffPtr) != NULL) return;

    struct CollisionStateDiff *diff = calloc(1, sizeof(struct CollisionStateDiff));
    if (diff == NULL) return;

    diff->head = NULL;

    (*diffPtr) = diff;
    diff = NULL;
}

extern void deallocCollisionStateDiff(struct CollisionStateDiff **diffPtr) {
    if (diffPtr == NULL || (*diffPtr) == NULL) return;

    struct CollisionStateDiff *diff = (*diffPtr);
    deallocCollisionStateDiffEntry(&diff->head);

    free(diff);
    diff = NULL;
    (*diffPtr) = NULL;
}
