#ifndef PY3DENGINE_GAME_OBJECT_H
#define PY3DENGINE_GAME_OBJECT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdbool.h>

struct ComponentListNode;
struct Py3dTransform;
struct ChildListNode;
struct String;
struct RenderingContext;
struct ResourceManager;
struct Py3dGameObject;
struct Py3dComponent;

struct GameObject {
    struct ComponentListNode *components;
    struct ChildListNode *children;
    struct GameObject *parent;
    struct String *name;
    struct Py3dTransform *transform;
    struct Py3dGameObject *pyGameObject;
};

extern bool PyInit_Py3dGameObject(PyObject *module);
extern bool Py3dGameObject_FindCtor(PyObject *module);
extern void Py3dGameObject_FinalizeCtor();
extern int Py3dGameObject_Check(PyObject *obj);
extern PyObject *Py3dGameObject_GetTransform(struct Py3dGameObject *self, PyObject *Py_UNUSED(ignored));
extern PyObject *Py3dGameObject_Update(struct Py3dGameObject *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dGameObject_Render(struct Py3dGameObject *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dGameObject_AttachChild(struct Py3dGameObject *self, PyObject *args, PyObject *kwds);

extern void allocGameObject(struct GameObject **gameObjectPtr);
extern void deleteGameObject(struct GameObject **gameObjectPtr);

extern void updateGameObject(struct GameObject *gameObject, float dt);
extern void renderGameObject(struct GameObject *gameObject, struct RenderingContext *renderingContext);

extern void removeChild(struct GameObject *gameObject, struct GameObject *target);
extern void removeChildByName(struct GameObject *gameObject, const char* name);

extern struct GameObject *findGameObjectByName(struct GameObject *gameObject, const char *name);

extern void attachPyComponent(struct GameObject *gameObject, struct Py3dComponent *newPyComponent);
extern struct BaseComponent *getGameObjectComponentByType(struct GameObject *gameObject, const char *typeName);
extern size_t getGameObjectComponentsLength(struct GameObject *gameObject);
extern struct Py3dComponent *getGameObjectComponentByIndex(struct GameObject *gameObject, size_t index);

extern struct String *getGameObjectName(struct GameObject *gameObject);
extern void setGameObjectName(struct GameObject *gameObject, const char *newName);

extern struct Py3dTransform *getGameObjectTransform(struct GameObject *gameObject);

#endif
