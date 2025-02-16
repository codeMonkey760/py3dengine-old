#ifndef PY3DENGINE_CONFIG_H
#define PY3DENGINE_CONFIG_H

#include <stdbool.h>
#include <stdio.h>

extern void parseConfig(FILE *config);
extern void parseConfigFile(const char *fileName);
extern void finalizeConfig();

extern int getConfigScreenWidth();
extern int getConfigScreenHeight();
extern int getConfigScreenLeft();
extern int getConfigScreenTop();
extern bool getConfigFullScreen();
extern int getConfigSwapInterval();
extern int getConfigMaxDynamicLights();
extern const char *getConfigStartingScene();
extern bool getConfigWfoReversePolygons();

#endif
