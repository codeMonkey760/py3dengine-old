#ifndef PY3DENGINE_WFO_PARSER_H
#define PY3DENGINE_WFO_PARSER_H

#include <stdio.h>

struct ResourceManager;

extern void importWaveFrontFile(struct ResourceManager *manager, const char *filePath);
extern void parseWaveFrontFile(struct ResourceManager *manager, FILE *wfo);
extern void importMaterialFile(struct ResourceManager *manager, const char *filePath);
extern void parseMaterialFile(struct ResourceManager *manager, FILE *mtl);

#endif
