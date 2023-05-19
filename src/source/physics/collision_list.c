#include <stdlib.h>

#include "physics/collision_list.h"

struct CollisionListEntry {
    struct Py3dCollider *key;
    struct Py3dCollider *value;
    struct CollisionListEntry *next;
};

static struct CollisionListEntry *findTableEnd(struct CollisionList *table) {
    if (table == NULL || table->head == NULL) return NULL;

    struct CollisionListEntry *curNode = table->head;
    while (curNode->next != NULL) {
        curNode = curNode->next;
    }

    return curNode;
}

static void allocCollisionTableEntry(struct CollisionListEntry **entryPtr) {
    if (entryPtr == NULL || (*entryPtr) != NULL) return;

    struct CollisionListEntry *entry = calloc(1, sizeof(struct CollisionListEntry));
    if (entry == NULL) return;

    entry->key = NULL;
    entry->value = NULL;
    entry->next = NULL;

    (*entryPtr) = entry;
    entry = NULL;
}

static void deallocCollisionTableEntry(struct CollisionListEntry **entryPtr) {
    if (entryPtr == NULL || (*entryPtr) == NULL) return;

    struct CollisionListEntry *entry = (*entryPtr);

    deallocCollisionTableEntry(&entry->next);
    entry->value = NULL;
    entry->key = NULL;

    free(entry);
    entry = NULL;
    (*entryPtr) = NULL;
}

void allocCollisionTable(struct CollisionList **tablePtr) {
    if (tablePtr == NULL || (*tablePtr) != NULL) return;

    struct CollisionList *table = calloc(1, sizeof(struct CollisionList));
    if (table == NULL) return;

    table->head = NULL;

    (*tablePtr) = table;
    table = NULL;
}

void deallocCollisionTable(struct CollisionList **tablePtr) {
    if (tablePtr == NULL || (*tablePtr) == NULL) return;

    struct CollisionList *table = (*tablePtr);
    deallocCollisionTableEntry(&table->head);

    free(table);
    table = NULL;
    (*tablePtr) = NULL;
}

void addCollisionToTable(struct CollisionList *table, struct Py3dCollider *key, struct Py3dCollider *value) {
    if (table == NULL || key == NULL || value == NULL) return;

    struct CollisionListEntry *curNode = table->head, *prevNode = NULL;
    while (curNode != NULL) {
        if (curNode->key == key && curNode->value == value) return;

        prevNode = curNode;
        curNode = curNode->next;
    }

    struct CollisionListEntry *newEntry = NULL;
    allocCollisionTableEntry(&newEntry);
    if (newEntry == NULL) return;

    newEntry->key = key;
    newEntry->value = value;

    if (prevNode == NULL) {
        table->head = newEntry;
    } else {
        prevNode->next = newEntry;
    }
}
