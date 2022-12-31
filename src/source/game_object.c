#include <stdlib.h>

#include "custom_string.h"
#include "logger.h"
#include "game_object.h"
#include "components/base_component.h"
#include "components/transform_component.h"
#include "components/component_factory.h"

struct ComponentListNode {
    struct BaseComponent *component;
    struct ComponentListNode *next;
};

struct ChildListNode {
    struct GameObject *child;
    struct ChildListNode *next;
};

static void deleteComponent(struct BaseComponent **componentPtr) {
    if (componentPtr == NULL || (*componentPtr) == NULL) return;

    struct BaseComponent *component = (*componentPtr);
    if (component->delete == NULL) {
        critical_log("%s", "[Game Object]: Tried to delete component with no virtual destructor. Memory leak certain.");
        return;
    }

    component->delete(componentPtr);
    component = NULL;
}

static void allocComponentListNode(struct ComponentListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) != NULL) return;

    struct ComponentListNode *newNode = calloc(1, sizeof(struct ComponentListNode));
    if (newNode == NULL) return;

    newNode->component = NULL;
    newNode->next = NULL;

    (*listNodePtr) = newNode;
    newNode = NULL;
}

static void deleteComponentListNode(struct ComponentListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) == NULL) return;

    struct ComponentListNode *node = (*listNodePtr);
    deleteComponent(&node->component);

    deleteComponentListNode(&node->next);

    free(node);
    node = NULL;
    (*listNodePtr) = NULL;
}

static void allocChildListNode(struct ChildListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) != NULL) return;

    struct ChildListNode *newNode = calloc(1, sizeof(struct ChildListNode));
    if (newNode == NULL) return;

    newNode->child = NULL;
    newNode->next = NULL;

    (*listNodePtr) = newNode;
    newNode = NULL;
}

static void deleteChildListNode(struct ChildListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) == NULL) return;

    struct ChildListNode *node = (*listNodePtr);
    deleteGameObject(&node->child);

    deleteChildListNode(&node->next);

    free(node);
    node = NULL;
    (*listNodePtr) = NULL;
}

void allocGameObject(struct GameObject **gameObjectPtr) {
    if (gameObjectPtr == NULL || (*gameObjectPtr) != NULL) return;

    struct GameObject *gameObject = NULL;
    gameObject = calloc(1, sizeof(struct GameObject));
    if (gameObject == NULL) return;

    gameObject->components = NULL;
    gameObject->children = NULL;
    gameObject->parent = NULL;
    gameObject->name = NULL;
    gameObject->transform = NULL;
    allocTransformComponent(&gameObject->transform);

    (*gameObjectPtr) = gameObject;
    gameObject = NULL;
}

void deleteGameObject(struct GameObject **gameObjectPtr) {
    if (gameObjectPtr == NULL || (*gameObjectPtr) == NULL) return;

    struct GameObject *gameObject = (*gameObjectPtr);
    deleteComponentListNode(&gameObject->components);
    deleteChildListNode(&gameObject->children);
    gameObject->parent = NULL;

    deleteString(&gameObject->name);
    deleteTransformComponent(&gameObject->transform);

    free(gameObject);
    gameObject = NULL;
    (*gameObjectPtr) = NULL;
}

void updateGameObject(struct GameObject *gameObject, float dt) {
    if (gameObject == NULL) return;

    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        struct BaseComponent *curComponent = curNode->component;
        if (curComponent != NULL && curComponent->update != NULL) {
            curComponent->update(curComponent, dt);
        }

        curNode = curNode->next;
    }

    struct ChildListNode *curChild = gameObject->children;
    while (curChild != NULL) {
        updateGameObject(curChild->child, dt);

        curChild = curChild->next;
    }
}

void renderGameObject(struct GameObject *gameObject, struct RenderingContext *renderingContext) {
    if (gameObject == NULL || renderingContext == NULL) return;

    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        struct BaseComponent *curComponent = curNode->component;
        if (curComponent != NULL && curComponent->render != NULL) {
            curComponent->render(curComponent, renderingContext);
        }

        curNode = curNode->next;
    }

    struct ChildListNode *curChild = gameObject->children;
    while (curChild != NULL) {
        renderGameObject(curChild->child, renderingContext);

        curChild = curChild->next;
    }
}

void resizeGameObject(struct GameObject *gameObject, int newWidth, int newHeight) {
    if (gameObject == NULL) return;

    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        struct BaseComponent *curComponent = curNode->component;
        if (curComponent != NULL && curComponent->resize != NULL) {
            curComponent->resize(curComponent, newWidth, newHeight);
        }

        curNode = curNode->next;
    }

    struct ChildListNode *curChild = gameObject->children;
    while (curChild != NULL) {
        resizeGameObject(curChild->child, newWidth, newHeight);

        curChild = curChild->next;
    }
}

void attachChild(struct GameObject *parent, struct GameObject *newChild) {
    if (parent == NULL || newChild == NULL) return;

    struct String *newChildName = getGameObjectName(newChild);
    if (newChildName == NULL) {
        error_log("%s", "Cannot attach child game object unless it has a name");
        return;
    }

    struct ChildListNode *prevNode = NULL, *curNode = parent->children;
    while (curNode != NULL) {
        if (stringEquals(getGameObjectName(curNode->child), newChildName)) {
            error_log("%s", "Cannot attach child game object unless its name is unique");
            return;
        }

        prevNode = curNode;
        curNode = curNode->next;
    }

    struct ChildListNode *newNode = NULL;
    allocChildListNode(&newNode);
    if (newNode == NULL) return;
    newNode->child = newChild;

    if (prevNode == NULL) {
        parent->children = newNode;
    } else {
        prevNode->next = newNode;
    }
    newChild->parent = parent;
}

void removeChild(struct GameObject *gameObject, struct GameObject *target) {
    critical_log("%s", "GameObject::removeChild is not yet implemented");

    // TODO: implement GameObject::removeChild
}

void removeChildByName(struct GameObject *gameObject, const char* name) {
    critical_log("%s", "GameObject::removeChildByName is not yet implemented");

    // TODO: implement GameObject::removeChildByName
}

struct GameObject *findGameObjectByName(struct GameObject *gameObject, const char *name) {
    if (gameObject == NULL || name == NULL) return NULL;

    if (stringEqualsCStr(gameObject->name, name)) {
        return gameObject;
    }

    struct ChildListNode *curNode = gameObject->children;
    struct GameObject *ret = NULL;
    while (curNode != NULL) {
        ret = findGameObjectByName(curNode->child, name);
        if (ret != NULL) {
            return ret;
        }

        curNode = curNode->next;
    }

    return ret;
}

void attachComponent(struct GameObject *gameObject, struct BaseComponent *newComponent) {
    if (gameObject == NULL || newComponent == NULL) return;

    struct String *newComponentName = getComponentName(newComponent);
    if (newComponentName == NULL) {
        error_log("%s", "Cannot attach component to game object unless it has a name");
        return;
    }

    struct ComponentListNode *prevNode = NULL, *curNode = gameObject->components;
    while (curNode != NULL) {
        if (stringEquals(getComponentName(curNode->component), newComponentName)) {
            error_log("%s", "Cannot attach component to game object unless its name is unique");
            return;
        }

        prevNode = curNode;
        curNode = curNode->next;
    }

    struct ComponentListNode *newNode = NULL;
    allocComponentListNode(&newNode);
    if (newNode == NULL) return;

    newNode->component = newComponent;

    if (prevNode == NULL) {
        gameObject->components = newNode;
    } else {
        prevNode->next = newNode;
    }
    newComponent->_owner = gameObject;
}

struct BaseComponent *getGameObjectComponentByType(struct GameObject *gameObject, const char *typeName) {
    if (gameObject == NULL || typeName == NULL) return NULL;

    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        if (stringEqualsCStr(getComponentTypeName(curNode->component), typeName)) {
            return curNode->component;
        }

        curNode = curNode->next;
    }

    return NULL;
}

void parseGameObject(json_object *json, struct GameObject *parent, struct GameObject **rootPtr) {
    if (json == NULL || rootPtr == NULL || (*rootPtr) != NULL) return;

    json_object *json_name = json_object_object_get(json, "name");
    if (json_name == NULL || !json_object_is_type(json_name, json_type_string)) {
        error_log("%s", "[GameObject]: Game Object must have a string property called \"name\"");
        return;
    }

    json_object *json_transform = json_object_object_get(json, "transform");
    if (json_transform == NULL || !json_object_is_type(json_transform, json_type_object)) {
        error_log("%s", "[GameObject]: Game Object must have an object property called \"transform\"");
        return;
    }

    json_object *json_components_array = json_object_object_get(json, "components");
    if (json_components_array == NULL || !json_object_is_type(json_components_array, json_type_array)) {
        error_log("%s", "[GameObject]: Game Object must have an array property called \"components\"");
        return;
    }

    json_object *json_children_array = json_object_object_get(json, "children");
    if (json_children_array == NULL || !json_object_is_type(json_children_array, json_type_array)) {
        error_log("%s", "[GameObject]: Game Object must have an array property called \"children\"");
        return;
    }

    struct GameObject *newGO = NULL;
    allocGameObject(&newGO);
    if (newGO == NULL) return;

    setGameObjectName(newGO, json_object_get_string(json_name));

    parseTransformComponent(json_transform, newGO->transform);

    size_t json_components_array_length = json_object_array_length(json_components_array);
    for (size_t i = 0; i < json_components_array_length; ++i) {
        json_object *cur_component_json = json_object_array_get_idx(json_components_array, i);
        if (cur_component_json == NULL || !json_object_is_type(cur_component_json, json_type_object)) {
            error_log(
                "[GameObject]: Could not parse component of Game Object with name \"%s\"",
                getGameObjectName(newGO)
            );

            continue;
        }

        json_object *type_name_json = json_object_object_get(cur_component_json, "type");
        if (type_name_json == NULL) {
            error_log("%s", "[GameObject]: Component must have string property with name \"type\"");

            continue;
        }

        struct BaseComponent *newComponent = NULL;
        componentFactoryCreateComponentFromJson(json_object_get_string(type_name_json), &newComponent);
        if (newComponent == NULL) {
            error_log("%s", "[GameObject]: Component failed to allocate during json parsing");

            continue;
        }
        if (newComponent->parse == NULL) {
            error_log("%s", "[GameObject]: Json parsing created a component with no virtual parse function");
            deleteComponent(&newComponent);

            continue;
        }

        if (!newComponent->parse(newComponent, cur_component_json)) {
            error_log("%s", "[GameObject]: Component failed to parse. Discarding it.");
            deleteComponent(&newComponent);

            continue;
        }

        attachComponent(newGO, newComponent);
        newComponent = NULL;
    }

    size_t json_children_array_length = json_object_array_length(json_children_array);
    for (size_t i = 0; i < json_children_array_length; ++i) {
        json_object *cur_child_json = json_object_array_get_idx(json_children_array, i);
        if (cur_child_json == NULL || !json_object_is_type(cur_child_json, json_type_object)) {
            error_log(
                "[GameObject]: Could not parse child of Game Object with name \"%s\"",
                getGameObjectName(newGO)
            );

            continue;
        }

        parseGameObject(cur_child_json, newGO, rootPtr);
    }

    if (parent != NULL) {
        attachChild(parent, newGO);
    } else {
        (*rootPtr) = newGO;
    }
    newGO = NULL;
}

struct String *getGameObjectName(struct GameObject *gameObject) {
    if (gameObject == NULL) return NULL;

    return gameObject->name;
}

void setGameObjectName(struct GameObject *gameObject, char *newName) {
    if (gameObject == NULL) return;

    if (gameObject->name == NULL) {
        allocString(&(gameObject->name), newName);
    } else {
        setChars(gameObject->name, newName);
    }
}

struct TransformComponent *getGameObjectTransform(struct GameObject *gameObject) {
    if (gameObject == NULL) return NULL;

    return gameObject->transform;
}
