#ifndef PY3DENGINE_IMPORTERS_SPRITE_SHEET_H
#define PY3DENGINE_IMPORTERS_SPRITE_SHEET_H

#include <json.h>
struct ResourceManager;

extern void importSprites(struct ResourceManager *manager, json_object *resourceDescriptor);

#endif
