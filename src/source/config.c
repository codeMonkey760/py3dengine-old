#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "logger.h"
#include "config.h"

#define LINE_BUFFER_SIZE_IN_ELEMENTS 64
#define NAME_BUFFER_SIZE_IN_ELEMENTS 64
#define VALUE_BUFFER_SIZE_IN_ELEMENTS 64

#define SCREEN_WIDTH_DEFAULT 1280
#define SCREEN_WIDTH_CONFIG_NAME "screen_width"
static unsigned int screen_width = SCREEN_WIDTH_DEFAULT;

#define SCREEN_HEIGHT_DEFAULT 720
#define SCREEN_HEIGHT_CONFIG_NAME "screen_height"
static unsigned int screen_height = SCREEN_HEIGHT_DEFAULT;

#define SCREEN_LEFT_DEFAULT 100
#define SCREEN_LEFT_CONFIG_NAME "screen_left"
static unsigned int screen_left = SCREEN_LEFT_DEFAULT;

#define SCREEN_TOP_DEFAULT 100
#define SCREEN_TOP_CONFIG_NAME "screen_top"
static unsigned int screen_top = SCREEN_TOP_DEFAULT;

#define FULL_SCREEN_DEFAULT false
#define FULL_SCREEN_CONFIG_NAME "full_screen"
static bool full_screen = FULL_SCREEN_DEFAULT;

#define SWAP_INTERVAL_DEFAULT 0
#define SWAP_INTERVAL_CONFIG_NAME "swap_interval"
static unsigned int swap_interval = SWAP_INTERVAL_DEFAULT;

static void clearCharBuffer(char *buffer, size_t numChars) {
    if (buffer == NULL || numChars == 0) return;

    memset(buffer, 0, sizeof(char) * numChars);
}

static char *advancePastSpaces(char *curPos) {
    if (curPos == NULL) return NULL;

    char curChar = (*curPos);
    while (curChar != 0 && isspace(curChar)) {
        curPos++;
        curChar = (*curPos);
    }

    return curPos;
}

static char *readStringIntoCharBuffer(char *buffer, char *src, size_t maxLenInElements) {
    if (buffer == NULL || src == NULL || maxLenInElements == 0) return src;

    char *curPosSrc = src;
    char *curPosBuffer = buffer;
    unsigned int count = 0;

    while ((*curPosSrc) != 0 && !isspace((*curPosSrc) && (*curPosSrc) != '=' && count < maxLenInElements)) {
        (*curPosBuffer) = (*curPosSrc);

        curPosBuffer++;
        curPosSrc++;
        count++;
    }

    return curPosSrc;
}

static char *consumeEquals(char *curPos) {
    if (curPos == NULL) return NULL;

    if ( (*curPos) == '=' ) {
        return curPos+1;
    } else {
        return curPos;
    }
}

static void storeValue(char *name, char *value) {
    if (strncmp(name, SCREEN_WIDTH_CONFIG_NAME, NAME_BUFFER_SIZE_IN_ELEMENTS) == 0) {
        screen_width = strtoul(value, NULL, 10);
    } else if (strncmp(name, SCREEN_HEIGHT_CONFIG_NAME, NAME_BUFFER_SIZE_IN_ELEMENTS) == 0) {
        screen_height = strtoul(value, NULL, 10);
    } else if (strncmp(name, SCREEN_LEFT_CONFIG_NAME, NAME_BUFFER_SIZE_IN_ELEMENTS) == 0) {
        screen_left = strtoul(value, NULL, 10);
    } else if (strncmp(name, SCREEN_TOP_CONFIG_NAME, NAME_BUFFER_SIZE_IN_ELEMENTS) == 0) {
        screen_top = strtoul(value, NULL, 10);
    } else if (strncmp(name, FULL_SCREEN_CONFIG_NAME, NAME_BUFFER_SIZE_IN_ELEMENTS) == 0) {
        full_screen = strncmp(value, "true", 4) == 0;
    } else if (strncmp(name, SWAP_INTERVAL_CONFIG_NAME, NAME_BUFFER_SIZE_IN_ELEMENTS) == 0) {
        swap_interval = strtoul(value, NULL, 10);
    } else {
        warning_log("[Config]: Unrecognized configuration setting \"%s\"", name);
        return;
    }

    trace_log("[Config]: Successfully set \"%s\" to %s", name, value);
}

void parseConfig(FILE *config) {
    if (config == NULL) return;

    char lineBuffer[LINE_BUFFER_SIZE_IN_ELEMENTS+1];
    clearCharBuffer(lineBuffer, LINE_BUFFER_SIZE_IN_ELEMENTS+1);
    char *curPos = lineBuffer;
    char nameBuffer[NAME_BUFFER_SIZE_IN_ELEMENTS+1];
    clearCharBuffer(nameBuffer, NAME_BUFFER_SIZE_IN_ELEMENTS+1);
    char valueBuffer[VALUE_BUFFER_SIZE_IN_ELEMENTS+1];
    unsigned int lineNumber = 0;

    while (fgets(lineBuffer, LINE_BUFFER_SIZE_IN_ELEMENTS, config)) {
        lineNumber++;

        if (strnlen(lineBuffer, LINE_BUFFER_SIZE_IN_ELEMENTS) < 2) continue;

        curPos = advancePastSpaces(curPos);
        if (*curPos == 0 || *curPos == '#') continue;

        clearCharBuffer(nameBuffer, NAME_BUFFER_SIZE_IN_ELEMENTS+1);
        curPos = readStringIntoCharBuffer(nameBuffer, curPos, NAME_BUFFER_SIZE_IN_ELEMENTS);

        curPos = advancePastSpaces(curPos);
        curPos = consumeEquals(curPos);
        curPos = advancePastSpaces(curPos);

        clearCharBuffer(valueBuffer, VALUE_BUFFER_SIZE_IN_ELEMENTS+1);
        readStringIntoCharBuffer(valueBuffer, curPos, VALUE_BUFFER_SIZE_IN_ELEMENTS);

        storeValue(nameBuffer, valueBuffer);
    }
}

void parseConfigFile(const char *fileName) {
    FILE *config = fopen(fileName, "r");
    if (config == NULL) {
        warning_log(
            "[Config]: Could not open \"%s\" for reading. Configuration will proceed with default values",
            fileName
        );
        return;
    }

    parseConfig(config);
    fclose(config);
}

unsigned int getConfigScreenWidth() {
    return screen_width;
}

unsigned int getConfigScreenHeight() {
    return screen_height;
}

unsigned int getConfigScreenLeft() {
    return screen_left;
}

unsigned int getConfigScreenTop() {
    return screen_top;
}

bool getConfigFullScreen() {
    return full_screen;
}

unsigned int getConfigSwapInterval() {
    return swap_interval;
}
