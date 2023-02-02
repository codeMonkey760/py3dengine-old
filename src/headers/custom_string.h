#ifndef PY3DENGINE_CUSTOM_STRING_H
#define PY3DENGINE_CUSTOM_STRING_H

#include <stdbool.h>

struct String {
    char *_c_str;
    unsigned long _len;
};

extern void allocString(struct String **stringPtr, const char *chars);
extern void deleteString(struct String **stringPtr);

extern bool stringEqualsCStr(struct String *s1, const char *s2);
extern bool stringEquals(struct String *s1, struct String *s2);

extern void stringConcatenate(struct String **stringPtrResult, struct String *s1, struct String *s2);

extern void setChars(struct String *string, const char *chars);

extern unsigned long getLength(struct String *string);
extern const char* getChars(struct String *string);

#endif
