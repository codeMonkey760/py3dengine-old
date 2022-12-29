#include "custom_string.h"
#include "resource_manager.h"
#include "logger.h"

#define TYPE_INVALID 0
#define TYPE_SHADER 1
#define TYPE_MODEL 2
#define TYPE_MATERIAL 3

struct ListNode {
    void *resource;
    unsigned int type;
    struct String *name;

    struct ListNode *next;
};

static void deleteResourceByType(void **resourcePtr, unsigned int type) {
    if (resourcePtr == NULL || (*resourcePtr == NULL) || type == TYPE_INVALID) return;

    if (type == TYPE_SHADER) {
        deleteShader((struct Shader **) resourcePtr);
    } else if (type == TYPE_MODEL) {
        deleteModel((struct Model **) resourcePtr);
    } else if (type == TYPE_MATERIAL) {
        deleteMaterial((struct Material **) resourcePtr);
    } else {
        critical_log("%s", "[Resource Manager]: Unable able to delete resource. Unrecognized type. Memory leak certain.");
    }
}

static void allocListNode(struct ListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) != NULL) return;

    struct ListNode *newNode = calloc(1, sizeof(struct ListNode));
    newNode->resource = NULL;
    newNode->type = TYPE_INVALID;
    newNode->name = NULL;

    (*listNodePtr) = newNode;
    newNode = NULL;
}

static void deleteListNode(struct ListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) == NULL) return;

    struct ListNode *listNode = (*listNodePtr);
    deleteResourceByType(&listNode->resource, listNode->type);
    listNode->type = 0;
    deleteString(&listNode->name);

    deleteListNode(&listNode->next);

    free(listNode);
    listNode = NULL;
    (*listNodePtr) = NULL;
}

static void storeResourceByType(
    struct ResourceManager *manager,
    void *resource,
    unsigned int type,
    struct String *name
) {
    if (manager == NULL || resource == NULL || type == TYPE_INVALID || name == NULL) return;

    struct ListNode *prevNode = NULL, *curNode = manager->_root;
    while (curNode != NULL) {
        if (curNode->type == type && stringEquals(curNode->name, name)) return;

        prevNode = curNode;
        curNode = curNode->next;
    }

    struct ListNode *newNode = NULL;
    allocListNode(&newNode);
    if (newNode == NULL) return;

    newNode->resource = resource;
    newNode->type = type;
    allocString(&newNode->name, getChars(name));
    newNode->next = NULL;

    if (prevNode == NULL) {
        manager->_root = newNode;
    } else {
        prevNode->next = newNode;
    }
}

static void * getResourceByType(struct ResourceManager *manager, const char *name, unsigned int type) {
    struct ListNode *curNode = manager->_root;
    while (curNode != NULL) {
        if (curNode->type == type && stringEqualsCStr(curNode->name, name)) {
            return curNode->resource;
        }

        curNode = curNode->next;
    }

    return NULL;
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

void storeShader(struct ResourceManager *manager, struct Shader *shader){
    if (manager == NULL || shader == NULL) return;

    struct String *shaderName = getShaderName(shader);
    if (shaderName == NULL) return;

    storeResourceByType(manager, shader, TYPE_SHADER, shaderName);
}

void storeModel(struct ResourceManager *manager, struct Model *model){
    if (manager == NULL || model == NULL) return;

    struct String *modelName = getModelName(model);
    if (modelName == NULL) return;

    storeResourceByType(manager, model, TYPE_MODEL, modelName);
}

void storeMaterial(struct ResourceManager *manager, struct Material *material){
    if (manager == NULL || material == NULL) return;

    struct String *materialName = getMaterialName(material);
    if (materialName == NULL) return;

    storeResourceByType(manager, material, TYPE_MATERIAL, materialName);
}

struct Shader * getShaderResource(struct ResourceManager *manager, const char *name){
    if (manager == NULL || name == NULL) return NULL;

    return (struct Shader *) getResourceByType(manager, name, TYPE_SHADER);
}

struct Model * getModelResource(struct ResourceManager *manager, const char *name){
    if (manager == NULL || name == NULL) return NULL;

    return (struct Model *) getResourceByType(manager, name, TYPE_MODEL);
}

struct Material * getMaterialResource(struct ResourceManager *manager, const char *name){
    if (manager == NULL || name == NULL) return NULL;

    return (struct Material *) getResourceByType(manager, name, TYPE_MATERIAL);
}