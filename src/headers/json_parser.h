#ifndef PY3DENGINE_JSON_PARSER_H
#define PY3DENGINE_JSON_PARSER_H

#include <stdbool.h>

struct GameObject;
struct ResourceManager;

typedef struct json_object json_object;

extern bool parseGameObject(
    json_object *jsonGameObject,
    struct GameObject *parent,
    struct GameObject **root,
    struct ResourceManager *resourceManager
);

#endif
