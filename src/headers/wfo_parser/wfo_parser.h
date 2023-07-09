#ifndef PY3DENGINE_WFO_PARSER_H
#define PY3DENGINE_WFO_PARSER_H

#include <stdio.h>

struct Py3dResourceManager;

extern void importWaveFrontFile(struct Py3dResourceManager *manager, const char *filePath);
extern void parseWaveFrontFile(struct Py3dResourceManager *manager, FILE *wfo);
extern void importMaterialFile(struct Py3dResourceManager *manager, const char *filePath);
extern void parseMaterialFile(struct Py3dResourceManager *manager, FILE *mtl);

#endif
