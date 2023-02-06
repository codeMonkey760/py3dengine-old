#ifndef PY3DENGINE_IMPORTERS_SHADER_H
#define PY3DENGINE_IMPORTERS_SHADER_H

struct Shader;
extern void importShader(struct Shader **shaderPtr, json_object *shaderDesc);

#endif
