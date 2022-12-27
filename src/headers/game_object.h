#ifndef PY3DENGINE_GAME_OBJECT_H
#define PY3DENGINE_GAME_OBJECT_H

struct ComponentListNode;
struct TransformComponent;
struct ChildListNode;
struct BaseComponent;
struct Camera;
struct String;

struct GameObject {
    struct ComponentListNode *components;
    struct ChildListNode *children;
    struct GameObject *parent;
    struct String *name;
    struct TransformComponent *transform;
};

extern void allocGameObject(struct GameObject **gameObjectPtr);
extern void deleteGameObject(struct GameObject **gameObjectPtr);

extern void updateGameObject(struct GameObject *gameObject, float dt);
extern void renderGameObject(struct GameObject *gameObject, struct Camera *camera);

extern void attachChild(struct GameObject *parent, struct GameObject *newChild);
extern void removeChild(struct GameObject *gameObject, struct GameObject *target);
extern void removeChildByName(struct GameObject *gameObject, const char* name);

extern struct GameObject *findGameObjectByName(struct GameObject *gameObject, const char *name);

extern void attachComponent(struct GameObject *gameObject, struct BaseComponent *newComponent);

extern struct String *getGameObjectName(struct GameObject *gameObject);
extern void setGameObjectName(struct GameObject *gameObject, char *newName);

extern struct TransformComponent *getGameObjectTransform(struct GameObject *gameObject);

#endif
