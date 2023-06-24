#ifndef PY3DENGINE_JSON_PARSER_H
#define PY3DENGINE_JSON_PARSER_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdbool.h>

struct Py3dGameObject;
struct ResourceManager;
struct Py3dScene;

typedef struct json_object json_object;

extern bool parseGameObject(
    json_object *jsonGameObject,
    struct Py3dGameObject *parent,
    struct Py3dGameObject **root,
    struct Py3dScene *scene,
    struct ResourceManager *resourceManager
);

#endif
