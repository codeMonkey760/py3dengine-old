#ifndef PY3DENGINE_IMPORTERS_TEXTURE_H
#define PY3DENGINE_IMPORTERS_TEXTURE_H

struct Texture;
extern void importTexture(struct Texture **texturePtr, json_object *textureDesc);

#endif
