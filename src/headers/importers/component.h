#ifndef PY3DENGINE_IMPORTERS_COMPONENT_H
#define PY3DENGINE_IMPORTERS_COMPONENT_H

struct PythonScript;
extern void importComponent(struct PythonScript **scriptPtr, const char *name);
extern void importBuiltinComponent(struct PythonScript **scriptPtr, const char *name);

#endif
