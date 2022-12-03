#include <string.h>
#include <stdlib.h>

#include "custom_string.h"

static void deleteCharBuffer(struct String *string) {
    if (string == NULL || string->_c_str == NULL) return;

    free(string->_c_str);
    string->_c_str = NULL;
    string->_len = 0;
}

void allocString(struct String **stringPtr, char *chars) {
    if (stringPtr == NULL || (*stringPtr) != NULL) return;

    struct String *string = NULL;
    string = calloc(1, sizeof(struct String));
    if (string == NULL) return;

    string->_c_str = NULL;
    string->_len = 0;

    setChars(string, chars);

    (*stringPtr) = string;
}

void deleteString(struct String **stringPtr) {
    if (stringPtr == NULL || (*stringPtr) == NULL) return;

    deleteCharBuffer((*stringPtr));

    free((*stringPtr));
    (*stringPtr) = NULL;
}

void setChars(struct String *string, char *chars) {
    if (string == NULL || chars == NULL) return;

    unsigned long num_chars = strlen(chars);
    if (num_chars == 0) return;

    char *newBuffer = calloc(num_chars + 1, sizeof(char));
    if (newBuffer == NULL) return;

    strncpy(newBuffer, chars, num_chars);
    newBuffer[num_chars] = 0;

    deleteCharBuffer(string);
    string->_c_str = newBuffer;
    string->_len = num_chars;
    newBuffer = NULL;
}

unsigned long getLength(struct String *string) {
    if (string == NULL) return -1;

    return string->_len;
}

char *getChars(struct String *string) {
    if (string == NULL) return NULL;

    return string->_c_str;
}