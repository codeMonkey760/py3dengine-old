#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
//#include <ini.h>

#include "logger.h"
#include "config.h"
#include "custom_string.h"

#define MAX_NAME_SIZE 64

#define SCREEN_WIDTH_DEFAULT 1280
#define SCREEN_WIDTH_CONFIG_NAME "screen_width"

#define SCREEN_HEIGHT_DEFAULT 720
#define SCREEN_HEIGHT_CONFIG_NAME "screen_height"

#define SCREEN_LEFT_DEFAULT 100
#define SCREEN_LEFT_CONFIG_NAME "screen_left"

#define SCREEN_TOP_DEFAULT 100
#define SCREEN_TOP_CONFIG_NAME "screen_top"

#define FULL_SCREEN_DEFAULT false
#define FULL_SCREEN_CONFIG_NAME "full_screen"

#define SWAP_INTERVAL_DEFAULT 0
#define SWAP_INTERVAL_CONFIG_NAME "swap_interval"

#define STARTING_SCENE_DEFAULT "default.json"
#define STARTING_SCENE_CONFIG_NAME "starting_scene"

struct Configuration {
    int screen_width;
    int screen_height;
    int screen_left;
    int screen_top;
    bool full_screen;
    int swap_interval;
    struct String *startingScene;
} config = {
    .screen_width = SCREEN_WIDTH_DEFAULT,
    .screen_height = SCREEN_HEIGHT_DEFAULT,
    .screen_left = SCREEN_LEFT_DEFAULT,
    .screen_top = SCREEN_TOP_DEFAULT,
    .full_screen = FULL_SCREEN_DEFAULT,
    .swap_interval = SWAP_INTERVAL_DEFAULT,
    .startingScene = NULL
};

static int handler(void *user, const char *section, const char *name, const char *value) {
    struct Configuration *local_config = (struct Configuration *) user;

    if (strncmp(name, SCREEN_WIDTH_CONFIG_NAME, MAX_NAME_SIZE) == 0) {
        local_config->screen_width = (int) strtol(value, NULL, 10);
    } else if (strncmp(name, SCREEN_HEIGHT_CONFIG_NAME, MAX_NAME_SIZE) == 0) {
        local_config->screen_height = (int) strtol(value, NULL, 10);
    } else if (strncmp(name, SCREEN_LEFT_CONFIG_NAME, MAX_NAME_SIZE) == 0) {
        local_config->screen_left = (int) strtol(value, NULL, 10);
    } else if (strncmp(name, SCREEN_TOP_CONFIG_NAME, MAX_NAME_SIZE) == 0) {
        local_config->screen_top = (int) strtol(value, NULL, 10);
    } else if (strncmp(name, FULL_SCREEN_CONFIG_NAME, MAX_NAME_SIZE) == 0) {
        local_config->full_screen = (bool) strncmp(value, "true", 4) == 0;
    } else if (strncmp(name, SWAP_INTERVAL_CONFIG_NAME, MAX_NAME_SIZE) == 0) {
        local_config->swap_interval = (int) strtol(value, NULL, 10);
    } else if (strncmp(name, STARTING_SCENE_CONFIG_NAME, MAX_NAME_SIZE) == 0) {
        if (local_config->startingScene == NULL) {
            allocString(&local_config->startingScene, value);
        } else {
            setChars(local_config->startingScene, value);
        }
    } else {
        warning_log("[Config]: Unrecognized configuration setting \"%s\"", name);
        return 0;
    }

    trace_log("[Config]: Successfully set \"%s\" to %s", name, value);
    return 1;
}

void parseConfig(FILE *configFile) {
//    if (ini_parse_file(configFile, handler, &config) < 0) {
//        error_log("%s", "[Config]: An error occurred while parsing configuration");
//    }

    return;
}

void parseConfigFile(const char *fileName) {
    FILE *configFile = fopen(fileName, "r");
    if (configFile == NULL) {
        warning_log(
            "[Config]: Could not open \"%s\" for reading. Configuration will proceed with default values",
            fileName
        );
        return;
    } else {
        trace_log("[Config]: Parsing \"%s\" for configuration values", fileName);
    }

    parseConfig(configFile);
    fclose(configFile);
}

void finalizeConfig() {
    deleteString(&config.startingScene);
}

int getConfigScreenWidth() {
    return config.screen_width;
}

int getConfigScreenHeight() {
    return config.screen_height;
}

int getConfigScreenLeft() {
    return config.screen_left;
}

int getConfigScreenTop() {
    return config.screen_top;
}

bool getConfigFullScreen() {
    return config.full_screen;
}

int getConfigSwapInterval() {
    return config.swap_interval;
}

const char *getConfigStartingScene() {
    if (config.startingScene == NULL) {
        return STARTING_SCENE_DEFAULT;
    } else {
        return getChars(config.startingScene);
    }
}
