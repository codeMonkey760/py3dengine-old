#include <stdbool.h>
#include <json-c/json.h>

#include "logger.h"
#include "config.h"
#include "custom_string.h"

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

#define MAX_DYNAMIC_LIGHTS_DEFAULT 8
#define MAX_DYNAMIC_LIGHTS_CONFIG_NAME "max_dynamic_lights"

#define STARTING_SCENE_DEFAULT "default.json"
#define STARTING_SCENE_CONFIG_NAME "starting_scene"

#define WFO_REVERSE_POLYGONS_DEFAULT true
#define WFO_REVERSE_POLYGONS_CONFIG_NAME "wfo_reverse_polygons"

struct Configuration {
    int screen_width;
    int screen_height;
    int screen_left;
    int screen_top;
    bool full_screen;
    int swap_interval;
    int max_dynamic_lights;
    struct String *startingScene;
    bool wfo_reverse_polygons;
} config = {
    .screen_width = SCREEN_WIDTH_DEFAULT,
    .screen_height = SCREEN_HEIGHT_DEFAULT,
    .screen_left = SCREEN_LEFT_DEFAULT,
    .screen_top = SCREEN_TOP_DEFAULT,
    .full_screen = FULL_SCREEN_DEFAULT,
    .swap_interval = SWAP_INTERVAL_DEFAULT,
    .max_dynamic_lights = MAX_DYNAMIC_LIGHTS_DEFAULT,
    .startingScene = NULL,
    .wfo_reverse_polygons = WFO_REVERSE_POLYGONS_DEFAULT
};

static json_object *get_object(json_object *parent, const char *key_name) {
    json_object *valueObj = json_object_object_get(parent, key_name);
    if (valueObj == NULL) {
        warning_log("[Config]: No value for \"%s\" was found in config", key_name);
    }

    return valueObj;
}

static bool check_type(json_object *obj, json_type expected_type) {
    json_type actual_type = json_object_get_type(obj);
    if (actual_type != expected_type) {
        warning_log(
            "[Config]: Expected \"%s\"  but found \"%s\" instead",
            json_type_to_name(expected_type),
            json_type_to_name(actual_type)
        );

        return false;
    }

    return true;
}

static void getIntFromObject(json_object *obj, const char *key_name, int *dst, int default_value) {
    if (obj == NULL || key_name == NULL || dst == NULL) return;

    json_object *valueObj = get_object(obj, key_name);
    if (valueObj == NULL) {
        (*dst) = default_value;
        return;
    }

    if (!check_type(valueObj, json_type_int)) {
        (*dst) = default_value;
        return;
    }

    int value = json_object_get_int(valueObj);
    (*dst) = value;
    trace_log("[Config]: Config value \"%s\" was successful set to \"%d\"", key_name, value);
}

static void getBoolFromObject(json_object *obj, const char *key_name, bool *dst, bool default_value) {
    if (obj == NULL || key_name == NULL || dst == NULL) return;

    json_object *valueObj = get_object(obj, key_name);
    if (valueObj == NULL) {
        (*dst) = default_value;
        return;
    }

    if (!check_type(valueObj, json_type_boolean)) {
        (*dst) = default_value;
        return;
    }

    bool value = json_object_get_boolean(valueObj);
    (*dst) = value;
    trace_log("[Config]: Config value \"%s\" was successful set to \"%s\"", key_name, (value) ? "true" : "false");
}

static void getStringFromObject(json_object *obj, const char *key_name, struct String *dst, const char *default_value) {
    if (obj == NULL || key_name == NULL || dst == NULL) return;

    json_object *valueObj = get_object(obj, key_name);
    if (valueObj == NULL) {
        setChars(dst, default_value);
        return;
    }

    if (!check_type(valueObj, json_type_string)) {
        setChars(dst, default_value);
        return;
    }

    const char *value = json_object_get_string(valueObj);
    setChars(dst, value);
    trace_log("[Config]: Config value \"%s\" was successful set to \"%s\"", key_name, value);
}

void parseConfig(FILE *configFile) {
    json_object *config_root = json_object_from_fd(fileno(configFile));
    if (config_root == NULL) {
        critical_log("[Config]: Could not parse config as JSON. It may not be valid. Default values will be used");
        return;
    }

    if (config.startingScene != NULL) {
        deleteString(&config.startingScene);
    }
    allocString(&config.startingScene, STARTING_SCENE_DEFAULT);

    trace_log("[Config]: Attempting to set \"%s\" from config", SCREEN_WIDTH_CONFIG_NAME);
    getIntFromObject(config_root, SCREEN_WIDTH_CONFIG_NAME, &config.screen_width, SCREEN_WIDTH_DEFAULT);
    trace_log("[Config]: Attempting to set \"%s\" from config", SCREEN_HEIGHT_CONFIG_NAME);
    getIntFromObject(config_root, SCREEN_HEIGHT_CONFIG_NAME, &config.screen_height, SCREEN_HEIGHT_DEFAULT);
    trace_log("[Config]: Attempting to set \"%s\" from config", SCREEN_LEFT_CONFIG_NAME);
    getIntFromObject(config_root, SCREEN_LEFT_CONFIG_NAME, &config.screen_left, SCREEN_LEFT_DEFAULT);
    trace_log("[Config]: Attempting to set \"%s\" from config", SCREEN_TOP_CONFIG_NAME);
    getIntFromObject(config_root, SCREEN_TOP_CONFIG_NAME, &config.screen_top, SCREEN_TOP_DEFAULT);
    trace_log("[Config]: Attempting to set \"%s\" from config", FULL_SCREEN_CONFIG_NAME);
    getBoolFromObject(config_root, FULL_SCREEN_CONFIG_NAME, &config.full_screen, FULL_SCREEN_DEFAULT);
    trace_log("[Config]: Attempting to set \"%s\" from config", SWAP_INTERVAL_CONFIG_NAME);
    getIntFromObject(config_root, SWAP_INTERVAL_CONFIG_NAME, &config.swap_interval, SWAP_INTERVAL_DEFAULT);
    trace_log("[Config]: Attempting to set \"%s\" from config", MAX_DYNAMIC_LIGHTS_CONFIG_NAME);
    getIntFromObject(config_root, MAX_DYNAMIC_LIGHTS_CONFIG_NAME, &config.swap_interval, MAX_DYNAMIC_LIGHTS_DEFAULT);
    trace_log("[Config]: Attempting to set \"%s\" from config", STARTING_SCENE_CONFIG_NAME);
    getStringFromObject(config_root, STARTING_SCENE_CONFIG_NAME, config.startingScene, STARTING_SCENE_DEFAULT);
    trace_log("[Config]: Attempting to set \"%s\" from config", WFO_REVERSE_POLYGONS_CONFIG_NAME);
    getBoolFromObject(config_root, WFO_REVERSE_POLYGONS_CONFIG_NAME, &config.wfo_reverse_polygons, WFO_REVERSE_POLYGONS_DEFAULT);

    json_object_put(config_root);
    config_root = NULL;
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

int getConfigMaxDynamicLights() {
    return config.max_dynamic_lights;
}

const char *getConfigStartingScene() {
    if (config.startingScene == NULL) {
        return STARTING_SCENE_DEFAULT;
    } else {
        return getChars(config.startingScene);
    }
}

bool getConfigWfoReversePolygons() {
    return config.wfo_reverse_polygons;
}
