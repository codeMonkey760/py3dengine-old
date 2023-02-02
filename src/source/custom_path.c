#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "custom_string.h"
#include "custom_path.h"

#define MAX_PATH_PART_SIZE 256

static void getWorkingDir(char *buffer) {
    if (buffer == NULL) return;

#ifdef _WIN32
    char sep = '\\';
#else
    char sep = '/';
#endif

#ifndef _WIN32
    getcwd(buffer, MAX_PATH_PART_SIZE);
#endif

    strcat(buffer, &sep);
}

static void correctPathSeparators(char *path) {
#ifdef _WIN32
    char *curPos = path;
    while (curPos != NULL && (*curPos) != 0) {
        if ((*curPos) == '/') {
            (*curPos) = sep;
        }

        curPos++;
    }
#endif
}

void createAbsolutePath(struct String **stringPtr, const char *relativePath) {
    if (stringPtr == NULL) return;

    deleteString(stringPtr);

    char basePath[MAX_PATH_PART_SIZE+1];
    memset(basePath, 0, MAX_PATH_PART_SIZE * sizeof(char));
    getWorkingDir(basePath);
    struct String *basePathStr = NULL;
    allocString(&basePathStr, basePath);

    if (relativePath == NULL) {
        (*stringPtr) = basePathStr;
        basePathStr = NULL;
        return;
    }

    struct String *relPathStr = NULL;
    allocString(&relPathStr, relativePath);
    correctPathSeparators(relPathStr->_c_str);

    stringConcatenate(stringPtr, basePathStr, relPathStr);
    deleteString(&basePathStr);
    deleteString(&relPathStr);
}