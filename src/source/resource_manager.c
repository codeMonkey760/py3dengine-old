#include <stdlib.h>

#include "custom_string.h"
#include "resource_manager.h"
#include "logger.h"

#include "resources/base_resource.h"

struct ListNode {
    struct BaseResource *resource;

    struct ListNode *next;
};

static void allocListNode(struct ListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) != NULL) return;

    struct ListNode *newNode = calloc(1, sizeof(struct ListNode));
    newNode->resource = NULL;

    (*listNodePtr) = newNode;
    newNode = NULL;
}

static void deleteListNode(struct ListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) == NULL) return;

    struct ListNode *listNode = (*listNodePtr);
    deleteResource(&listNode->resource);

    deleteListNode(&listNode->next);

    free(listNode);
    listNode = NULL;
    (*listNodePtr) = NULL;
}

void allocResourceManager(struct ResourceManager **resourceManagerPtr){
    if (resourceManagerPtr == NULL || (*resourceManagerPtr) != NULL) return;

    struct ResourceManager *newResourceManager = calloc(1, sizeof(struct ResourceManager));
    if (newResourceManager == NULL) return;

    newResourceManager->_root = NULL;

    (*resourceManagerPtr) = newResourceManager;
    newResourceManager = NULL;
}

void deleteResourceManager(struct ResourceManager **resourceManagerPtr){
    if (resourceManagerPtr == NULL || (*resourceManagerPtr) == NULL) return;

    struct ResourceManager *manager = (*resourceManagerPtr);

    deleteListNode(&manager->_root);

    free(manager);
    manager = NULL;
    (*resourceManagerPtr) = NULL;
}

void storeResource(struct ResourceManager *manager, struct BaseResource *resource) {
    if (manager == NULL || resource == NULL) return;

    struct ListNode *prevNode = NULL, *curNode = manager->_root;
    while (curNode != NULL) {
        if (resourceNamesEqual(curNode->resource, resource)) {
            error_log(
                "[ResourceManager]: A name collision occurred while storing resource named \"%s\". The resource will likely be leaked.",
                getChars(getResourceName(resource))
            );
            return;
        }

        prevNode = curNode;
        curNode = curNode->next;
    }

    struct ListNode *newNode = NULL;
    allocListNode(&newNode);
    if (newNode == NULL) return;

    newNode->resource = resource;
    newNode->next = NULL;

    if (prevNode == NULL) {
        manager->_root = newNode;
    } else {
        prevNode->next = newNode;
    }
}

struct BaseResource *getResource(struct ResourceManager *manager, const char *name) {
    struct ListNode *curNode = manager->_root;
    while (curNode != NULL) {
        struct BaseResource *curResource = curNode->resource;
        if (curResource == NULL) {
            critical_log("%s", "[ResourceManager]: NULL pointer detected in resource list. Memory leak likely.");
            continue;
        }

        if (stringEqualsCStr(curResource->_name, name)) {
            return curNode->resource;
        }

        curNode = curNode->next;
    }

    return NULL;
}
