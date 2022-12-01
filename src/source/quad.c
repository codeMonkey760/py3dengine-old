#include <stdlib.h>

#include "util.h"
#include "shader.h"
#include "quadmodel.h"
#include "quad.h"

void allocQuad(struct Quad **quadPtr) {
    if (quadPtr == NULL || (*quadPtr != NULL)) {
        return;
    }

    struct Quad *newQuad = calloc(1, sizeof(struct Quad));
    if (newQuad == NULL) {
        return;
    }

    Vec3Identity(newQuad->_posW);
    newQuad->_posW[3] = 1.0f;
    QuaternionIdentity(newQuad->_orientation);
    Vec3Identity(newQuad->_scale);
    newQuad->_cycle_rate = M_PI_2;
    newQuad->_theta = 0.0f;

    Mat4Identity(newQuad->wMtxCache);
    newQuad->wMtxCacheDirty = true;

    (*quadPtr) = newQuad;
}

void deleteQuad(struct Quad **quadPtr) {
    if (quadPtr == NULL || (*quadPtr) == NULL) {
        return;
    }

    free((*quadPtr));
    (*quadPtr) = NULL;
}

void updateQuad(struct Quad *quad, float dt) {
    if (quad == NULL) return;

    quad->_theta += dt * quad->_cycle_rate;
    quad->_theta = clampRadians(quad->_theta);

    quad->_diffuseColor[0] = (cosf(quad->_theta) * 0.5f) + 0.5f;
    quad->_diffuseColor[1] = 0.0f;
    quad->_diffuseColor[2] = (sinf(quad->_theta) * 0.5f) + 0.5f;
    quad->_diffuseColor[3] = 1.0f;
}

void renderQuad(struct Quad *quad) {
    if (quad == NULL) return;

    enableShader();
    setDiffuseColor(quad->_diffuseColor);

    bindQuadModel();
    renderQuadModel();
    unbindQuadModel();

    disableShader();
}

void setPosW(struct Quad *quad, float newPosW[3]) {
    if (quad == NULL) return;

    Vec3Copy(quad->_posW, newPosW);
    quad->wMtxCacheDirty = true;
}

void setOrientation(struct Quad *quad, float newOrientation[4]) {
    if (quad == NULL) return;

    QuaternionCopy(quad->_orientation, newOrientation);
    quad->wMtxCacheDirty = true;
}

void setScale(struct Quad *quad, float newScale[3]) {
    if (quad == NULL) return;

    Vec3Copy(quad->_scale, newScale);
    quad->wMtxCacheDirty = true;
}
