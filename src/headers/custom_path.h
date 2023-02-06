#ifndef PY3DENGINE_CUSTOM_PATH_H
#define PY3DENGINE_CUSTOM_PATH_H

struct String;

extern void createAbsolutePath(struct String **stringPtr, const char *relativePath);
extern void pathConcatenate(struct String **pathResultPtr, struct String *p1, struct String *p2);

#endif
