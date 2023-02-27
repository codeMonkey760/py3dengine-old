#ifndef PY3DENGINE_IMPORTERS_COMPONENT_H
#define PY3DENGINE_IMPORTERS_COMPONENT_H

#include <json.h>

struct PythonScript;
extern void importComponent(struct PythonScript **scriptPtr, json_object *componentDesc);
extern void importBuiltinComponent(struct PythonScript **scriptPtr, const char *name);

#endif
