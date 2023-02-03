#include <string.h>
#include <stdlib.h>

#include "custom_string.h"

static void deleteCharBuffer(struct String *string) {
    if (string == NULL || string->_c_str == NULL) return;

    free(string->_c_str);
    string->_c_str = NULL;
    string->_len = 0;
}

static void allocEmptyString(struct String **stringPtr) {
    if (stringPtr == NULL || (*stringPtr) != NULL) return;

    struct String *newString = NULL;
    newString = calloc(1, sizeof(struct String));
    if (newString == NULL) return;

    newString->_c_str = NULL;
    newString->_len = 0;

    (*stringPtr) = newString;
    newString = NULL;
}

void allocString(struct String **stringPtr, const char *chars) {
    if (stringPtr == NULL || (*stringPtr) != NULL) return;

    struct String *newString = NULL;
    allocEmptyString(&newString);
    if (newString == NULL) return;

    setChars(newString, chars);

    (*stringPtr) = newString;
    newString = NULL;
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

void stringConcatenate(struct String **stringPtrResult, struct String *s1, struct String *s2) {
    if (stringPtrResult == NULL || (*stringPtrResult) != NULL || s1 == NULL || s2 == NULL) return;

    size_t s1_len = strlen(s1->_c_str);
    size_t s2_len = strlen(s2->_c_str);
    size_t resSize = s1_len + s2_len;

    char *newBuffer = calloc(resSize + 1, sizeof(char));
    if (newBuffer == NULL) return;

    strncpy(newBuffer, s1->_c_str, s1_len);
    strncat(newBuffer, s2->_c_str, s2_len);

    struct String *result = NULL;
    allocEmptyString(&result);
    if (result == NULL) {
        free(newBuffer);
        newBuffer = NULL;
        return;
    }

    result->_c_str = newBuffer;
    newBuffer = NULL;
    result->_len = resSize;

    (*stringPtrResult) = result;
    result = NULL;
}

void setChars(struct String *string, const char *chars) {
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

const char *getChars(struct String *string) {
    if (string == NULL) return NULL;

    return string->_c_str;
}