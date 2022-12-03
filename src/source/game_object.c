#include <stdlib.h>

#include "custom_string.h"
#include "game_object.h"

void allocGameObject(struct GameObject **gameObjectPtr) {
    if (gameObjectPtr == NULL || (*gameObjectPtr) != NULL) return;

    struct GameObject *gameObject = NULL;
    gameObject = calloc(1, sizeof(struct GameObject));
    if (gameObject == NULL) return;

    gameObject->components = NULL;
    gameObject->children = NULL;
    gameObject->parent = NULL;
    gameObject->name = NULL;

    (*gameObjectPtr) = gameObject;
    gameObject = NULL;
}

void deleteGameObject(struct GameObject **gameObjectPtr) {
    if (gameObjectPtr == NULL || (*gameObjectPtr) == NULL) return;

    struct GameObject *gameObject = (*gameObjectPtr);
    // TODO: should Game Objects delete their components? ... they dont alloc them
    // TODO: so ... do Game Objects delete their children? ... they dont alloc them
    /* In previous projects I've had a special class that constructs and tears down scene graphs */
    /* But what about situational deletion rather than scene boot and tear down? */
    // TODO: in the far far future Game Objects may emit events when detached from the scene graph
    // TODO: does setting the parent NULL here or deleting the Game Object count in that situation?
    gameObject->parent = NULL;

    deleteString(&(gameObject->name));

    free(gameObject);
    gameObject = NULL;
    (*gameObjectPtr) = NULL;
}

char *getGameObjectName(struct GameObject *gameObject) {
    if (gameObject == NULL) return NULL;

    return getChars(gameObject->name);
}

void setGameObjectName(struct GameObject *gameObject, char *newName) {
    if (gameObject == NULL) return;

    if (gameObject->name == NULL) {
        allocString(&(gameObject->name), newName);
    } else {
        setChars(gameObject->name, newName);
    }
}