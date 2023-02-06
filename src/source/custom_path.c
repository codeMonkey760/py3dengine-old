#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#ifndef _WIN32
static const char sep[] = {'/', 0};
#else
static const char sep[] = {'\\', 0};
#endif

#include "custom_string.h"
#include "custom_path.h"

#define MAX_PATH_PART_SIZE 256

static void getWorkingDir(char *buffer) {
    if (buffer == NULL) return;

#ifndef _WIN32
    getcwd(buffer, MAX_PATH_PART_SIZE);
#else
    GetCurrentDirectoryA(MAX_PATH_PART_SIZE, buffer);
#endif
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

void pathConcatenate(struct String **pathResultPtr, struct String *p1, struct String *p2) {
    if (pathResultPtr == NULL || (*pathResultPtr) != NULL || p1 == NULL || p2 == NULL) return;

    struct String *fsSep = NULL;
    allocString(&fsSep, sep);

    struct String *step1 = NULL;
    stringConcatenate(&step1, p1, fsSep);

    stringConcatenate(pathResultPtr, step1, p2);
    deleteString(&fsSep);
    deleteString(&step1);
}
