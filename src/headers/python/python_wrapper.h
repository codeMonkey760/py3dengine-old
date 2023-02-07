#ifndef PY3DENGINE_PYTHON_WRAPPER_H
#define PY3DENGINE_PYTHON_WRAPPER_H

#include <stdbool.h>

extern bool initializePython(int argc, char **argv);
extern void appendImportPath(const char *relPath);
extern void finalizePython();

#endif
