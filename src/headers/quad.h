#ifndef PY3DENGINE_QUAD_H
#define PY3DENGINE_QUAD_H

#include <stdbool.h>

#include "camera.h"

struct Quad {
    float _posW[4]; //world space
    float _orientation[4]; //quaternion
    float _scale[3]; //world space
    float _diffuseColor[4];
    float _cycle_rate;
    float _theta;

    float wMtxCache[16]; //cache for world mtx
    bool wMtxCacheDirty;
};

extern void allocQuad(struct Quad **quadPtr);
extern void deleteQuad(struct Quad **quadPtr);

extern void updateQuad(struct Quad *quad, float dt);
extern void renderQuad(struct Quad *quad, struct Camera *camera);

extern void setPosW(struct Quad *quad, float newPosW[3]);
extern void setOrientation(struct Quad *quad, float newOrientation[4]);
extern void setScale(struct Quad *quad, float newScale[3]);

#endif
