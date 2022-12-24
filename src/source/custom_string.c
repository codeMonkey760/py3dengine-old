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

bool stringEqualsCStr(struct String *s1, const char *s2) {
    if (s1 == NULL || s2 == NULL) return false;
    if (s1->_len == 0) return false;
    if (s1->_c_str == NULL) return false;

    unsigned long s2len = strlen(s2);
    if (s1->_len != s2len) return false;

    return strncmp(s1->_c_str, s2, s1->_len) == 0;
}

bool stringEquals(struct String *s1, struct String *s2) {
    if (s1 == NULL || s2 == NULL) return false;
    if (s1->_len == 0 || s2->_len == 0) return false;
    if (s1->_c_str == NULL || s2->_c_str == NULL) return false;

    if (s1->_len != s2->_len) return false;
    return strncmp(s1->_c_str, s2->_c_str, s1->_len) == 0;
}

void setChars(struct String *string, char *chars) {
    if (string == NULL || chars == NULL) return;

    unsigned long num_chars = strlen(chars);
    if (num_chars == 0) return;

    char *newBuffer = calloc(num_chars + 1, sizeof(char));
    if (newBuffer == NULL) return;

    memset(newBuffer, 0, (num_chars + 1) * sizeof(char));
    strncpy(newBuffer, chars, num_chars);

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