#ifndef PY3DENGINE_SHADER_H
#define PY3DENGINE_SHADER_H

#include "resources/base_resource.h"

#define RESOURCE_TYPE_NAME_SHADER "Shader"

struct Shader {
    struct BaseResource _base;

    unsigned int _vertexShader;
    unsigned int _fragShader;
    unsigned int _program;

    int _diffuseColorLoc;
    int _cameraPositionLoc;
    int _wMtxLoc;
    int _witMtxLoc;
    int _wvpMtxLoc;
};

extern bool isResourceTypeShader(struct BaseResource *resource);

extern void allocShader(struct Shader **shaderPtr);
extern void deleteShader(struct Shader **shaderPtr);

extern void initShader(struct Shader *shader, const char *vertexShaderSource, const char *fragShaderSource);
extern void enableShader(struct Shader *shader);
extern void disableShader(struct Shader *shader);

extern void setDiffuseColor(struct Shader *shader, const float newDiffuseColor[3]);
extern void setCameraPosition(struct Shader *shader, float newCameraPos[3]);
extern void setWMtx(struct Shader *shader, float newWMtx[16]);
extern void setWITMtx(struct Shader *shader, float newWITMtx[16]);
extern void setWVPMtx(struct Shader *shader, float newWVPMtx[16]);

#endif
