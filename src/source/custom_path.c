#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "custom_string.h"
#include "custom_path.h"

#define MAX_PATH_PART_SIZE 256

static void getWorkingDir(char *buffer) {
    if (buffer == NULL) return;

#ifdef _WIN32
    char sep[] = {'\\', 0};
#else
    char sep[] = {'/', 0};
#endif

#ifndef _WIN32
    getcwd(buffer, MAX_PATH_PART_SIZE);
#else
    GetCurrentDirectoryA(MAX_PATH_PART_SIZE, buffer);
#endif

    strcat(buffer, sep);
}

static void correctPathSeparators(char *path) {
#ifdef _WIN32
    char *curPos = path;
    while (curPos != NULL && (*curPos) != 0) {
        if ((*curPos) == '/') {
            (*curPos) = '\\';
        }

        curPos++;
    }
#endif
}

void createAbsolutePath(struct String **stringPtr, const char *relativePath) {
    if (stringPtr == NULL) return;

    deleteString(stringPtr);

    char basePath[MAX_PATH_PART_SIZE+2];
    memset(basePath, 0, MAX_PATH_PART_SIZE+2 * sizeof(char));
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