#ifndef PY3DENGINE_WFO_PARSER_H
#define PY3DENGINE_WFO_PARSER_H

#include <stdio.h>

#include "model.h"

extern void parseWaveFrontFile(FILE *wfo, struct Model **modelPtr);

#endif
