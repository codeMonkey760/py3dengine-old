#ifndef PY3DENGINE_CUSTOM_STRING_H
#define PY3DENGINE_CUSTOM_STRING_H

struct String {
    char *_c_str;
    unsigned long _len;
};

extern void allocString(struct String **stringPtr, char *chars);
extern void deleteString(struct String **stringPtr);

extern void setChars(struct String *string, char *chars);

extern unsigned long getLength(struct String *string);
extern char* getChars(struct String *string);

#endif
