#include <stdlib.h>

#include "logger.h"
#include "util.h"
#include "quadmodel.h"
#include "shader.h"
#include "engine.h"

static void updateStats(struct Engine *engine, float dt) {
    static float since_last_calc = 0.0f;
    static int frame_count = 0;

    if (engine == NULL) return;

    engine->_elapsed_time += dt;
    frame_count++;
    since_last_calc += dt;

    if (engine->_print_report == true && since_last_calc >= 1.0f) {
        float ms = (since_last_calc / (float) frame_count) * 1000.0f;
        float fps = (float) frame_count;

        frame_count = 0;
        while (since_last_calc >= 1.0f) {
            since_last_calc -= 1.0f;
        }

        engine->_fps = fps;
        engine->_mpf = ms;
    }
}

static void printStats(struct Engine *engine, float dt) {
    if (engine == NULL) return;

    engine->_time_since_last_report += dt;
    if (engine->_time_since_last_report >= 1.0f) {
        engine->_time_since_last_report = clampValue(engine->_time_since_last_report, 1.0f);

        if (engine->_print_report == true) {
            trace_log("UP_TIME: %.2f FPS %.2f MS %.2f", engine->_elapsed_time, engine->_fps, engine->_mpf);
        }
    }
}

void allocEngine(struct Engine **enginePtr){
    if (enginePtr == NULL || (*enginePtr) != NULL) return;

    struct Engine *engine = calloc(1, sizeof(struct Engine));
    if (engine == NULL) return;

    engine->_fps = 0.0f;
    engine->_mpf = 0.0f;
    engine->_elapsed_time = 0.0f;
    engine->_time_since_last_report = 0.0f;
    engine->_print_report = true;

    engine->quad[0] = NULL;
    engine->quad[1] = NULL;
    engine->camera = NULL;

    (*enginePtr) = engine;
}

void deleteEngine(struct Engine **enginePtr){
    if (enginePtr == NULL || (*enginePtr) == NULL) return;

    struct Engine *engine = (*enginePtr);
    deleteQuad(&(engine->quad[0]));
    deleteQuad(&(engine->quad[1]));
    deleteCamera(&(engine->camera));
    engine = NULL;

    deleteQuadModel();
    deleteShader();

    free((*enginePtr));
    (*enginePtr) = NULL;
}

void initEngine(struct Engine *engine){
    if (engine == NULL) return;

    initShader();
    initQuadModel();

    struct Quad *curQuad = NULL;
    float posW[3] = {0.0f};
    float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    posW[0] = -2.0f;
    color[0] = 0.8f;
    color[2] = 0.2f;
    allocQuad(&curQuad);
    setPosWQuad(curQuad, posW);
    setDiffuseColorQuad(curQuad, color);
    engine->quad[0] = curQuad;
    curQuad = NULL;

    posW[0] = 2.0f;
    color[0] = 0.2f;
    color[2] = 0.8f;
    allocQuad(&curQuad);
    setPosWQuad(curQuad, posW);
    setDiffuseColorQuad(curQuad, color);
    engine->quad[1] = curQuad;
    curQuad = NULL;

    struct Camera *camera = NULL;
    allocCamera(&camera);
    engine->camera = camera;
    camera = NULL;
}

void updateEngine(struct Engine *engine, float dt){
    if (engine == NULL) return;

    updateStats(engine, dt);
    printStats(engine, dt);

    updateQuad(engine->quad[0], dt);
    updateQuad(engine->quad[1], dt);
    updateCamera(engine->camera, dt);
}

void renderEngine(struct Engine *engine){
    if (engine == NULL) return;

    renderQuad(engine->quad[0], engine->camera);
    renderQuad(engine->quad[1], engine->camera);
}
