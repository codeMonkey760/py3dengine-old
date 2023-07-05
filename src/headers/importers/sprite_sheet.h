#ifndef PY3DENGINE_IMPORTERS_SPRITE_SHEET_H
#define PY3DENGINE_IMPORTERS_SPRITE_SHEET_H

#include <json.h>
struct Py3dResourceManager;

extern void importSprites(struct Py3dResourceManager *manager, json_object *resourceDescriptor);

#endif
