#ifndef PY3DENGINE_GAME_OBJECT_H
#define PY3DENGINE_GAME_OBJECT_H

struct GameObject {
    struct ComponentListNode *components;
    struct GameObject *children;
    struct GameObject *parent;
    struct String *name;
};

extern void allocGameObject(struct GameObject **gameObjectPtr);
extern void deleteGameObject(struct GameObject **gameObjectPtr);

extern void updateGameObject(struct GameObject *gameObject);
extern void renderGameObject(struct GameObject *gameObject);

extern void addChild(struct GameObject *gameObject, struct GameObject *newChild);
extern void removeChild(struct GameObject *gameObject, struct GameObject *target);
extern void removeChildByName(struct GameObject *gameObject, const char* name);

extern char* getGameObjectName(struct GameObject *gameObject);
extern void setGameObjectName(struct GameObject *gameObject, char *newName);

#endif
