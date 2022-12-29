#ifndef PY3DENGINE_CONFIG_H
#define PY3DENGINE_CONFIG_H

#include <stdbool.h>
#include <stdio.h>

extern void parseConfig(FILE *config);
extern void parseConfigFile(const char *fileName);

extern unsigned int getConfigScreenWidth();
extern unsigned int getConfigScreenHeight();
extern unsigned int getConfigScreenLeft();
extern unsigned int getConfigScreenTop();
extern bool getConfigFullScreen();
extern unsigned int getConfigSwapInterval();

#endif
